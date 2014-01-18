#include "decoder.h"
#include "controller.h"

#include <thread/blocklock.h>
#include <filter/resample.h>

#include <zeppelin/logger.h>

using player::Decoder;

// =====================================================================================================================
Decoder::Decoder(size_t bufferSize,
		 const Format& outputFormat,
		 Fifo& fifo,
		 ControllerImpl& ctrl,
		 const config::Config& config)
    : m_bufferSize(bufferSize),
      m_fifo(fifo),
      m_format(0, 0),
      m_outputFormat(outputFormat),
      m_resampling(false),
      m_ctrl(ctrl),
      m_config(config)
{
}

// =====================================================================================================================
void Decoder::setInput(const std::shared_ptr<codec::BaseCodec>& input)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<Input>(input));
    m_cond.signal();
}

// =====================================================================================================================
void Decoder::startDecoding()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(START));
    m_cond.signal();
}

// =====================================================================================================================
void Decoder::stopDecoding()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(STOP));
    m_cond.signal();

    // wait until all of the commands are processed, so the caller can make sure that decoding is really stopped
    while (!m_commands.empty())
	m_emptyCond.wait(m_mutex);
}

// =====================================================================================================================
void Decoder::seek(off_t seconds)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<Seek>(seconds));
    m_cond.signal();
}

// =====================================================================================================================
void Decoder::notify()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(NOTIFY));
    m_cond.signal();
}

// =====================================================================================================================
void Decoder::run()
{
    // true when the loop should wait for commands
    bool wait = true;

    bool working = false;

    while (1)
    {
	m_mutex.lock();

	// wait until we have some command to process
	while (wait && m_commands.empty())
	    m_cond.wait(m_mutex);

	// reset wait to true here ...
	wait = true;

	while (!m_commands.empty())
	{
	    std::shared_ptr<CmdBase> cmd = m_commands.front();
	    m_commands.pop_front();

	    switch (cmd->m_cmd)
	    {
		case INPUT :
		    LOG("decoder: input");

		    // before changing input check whether we performed resampling for the previous file because in that
		    // case the resampler must be removed from the filters
		    if (m_resampling)
		    {
			// here we assume that the resampler is the last filter, it is true for now ... :)
			m_filters.pop_back();
			m_resampling = false;
		    }

		    m_input = static_cast<Input&>(*cmd).m_input;

		    if (m_input)
		    {
			m_format = m_input->getFormat();

			// check whether we need to perform resampling
			if (m_format.getRate() != m_outputFormat.getRate())
			    turnOnResampling();
		    }

		    break;

		case START :
		    LOG("decoder: start");

		    if (!m_input)
		    {
			LOG("decoder: unable to start working without input!");
			break;
		    }

		    working = true;

		    break;

		case STOP :
		    LOG("decoder: stop");
		    m_fifo.reset();
		    working = false;
		    break;

		case SEEK :
		    LOG("decoder: seek");

		    if (!m_input)
		    {
			LOG("decoder: unable to seek without input!");
			break;
		    }

		    if (working)
		    {
			LOG("decoder: tried to seek without stopping");
			break;
		    }

		    m_input->seek(static_cast<Seek&>(*cmd).m_seconds * m_format.getRate());

		    break;

		case NOTIFY :
		    // do nothing here, this command is sent to wake up the decoder
		    break;
	    }
	}

	m_emptyCond.signal();

	m_mutex.unlock();

	// do nothing if we are not working
	if (!working || !m_input)
	    continue;

	size_t fifoSize = m_fifo.getBytes();

	if (fifoSize >= m_bufferSize)
	    continue;

	// calculate the minimum size of the data we must put into the buffer
	size_t minSize = m_bufferSize - fifoSize;

	float* samples;
	size_t count;

	if (!m_input->decode(samples, count))
	{
	    // the end of the stream has been reached
	    LOG("decoder: end of stream");
	    m_fifo.addMarker();

	    // remove the input of the decoder
	    m_input.reset();

	    // let the controller know that the decoder finished working
	    m_ctrl.command(ControllerImpl::DECODER_FINISHED);

	    continue;
	}

	// perform filters on the decoded samples
	runFilters(samples, count, m_format);

	// calculate the size of the decoded samples
	size_t size = m_format.sizeOfSamples(count);

	// put them into the fifo
	m_fifo.addSamples(samples, size);

	if (size < minSize)
	{
	    // do not wait for a command in case of we did not fill the fifo in this round
	    wait = false;
	}
    }
}

// =====================================================================================================================
void Decoder::runFilters(float*& samples, size_t& count, const Format& format)
{
    // process all filters
    for (const auto& filter : m_filters)
    {
	try
	{
	    filter->run(samples, count, format);
	}
	catch (const filter::FilterException& e)
	{
	    LOG("decoder: filter error: " << e.what());
	}
    }

    // make sure samples are still in the valid -1.0 ... 1.0 range
    for (size_t i = 0; i < count * format.getChannels(); ++i)
    {
	float& s = samples[i];

	if (s > 1.0f)
	    s = 1.0f;
	else if (s < -1.0f)
	    s = -1.0f;
    }
}

// =====================================================================================================================
void Decoder::turnOnResampling()
{
    LOG("decoder: turning on resampling (src=" <<
	m_format.getRate() <<
	", dst=" <<
	m_outputFormat.getRate() <<
	")");

    std::shared_ptr<filter::BaseFilter> resampler =
	std::make_shared<filter::Resample>(m_format.getRate(), m_outputFormat.getRate(), m_config);

    try
    {
	resampler->init();
	m_filters.push_back(resampler);
	m_resampling = true;
    }
    catch (const filter::FilterException& e)
    {
	LOG("decoder: unable to initialize resampler: " << e.what());
    }
}
