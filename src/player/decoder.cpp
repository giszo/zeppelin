#include "decoder.h"
#include "controller.h"

#include <thread/blocklock.h>

#include <iostream>

using player::Decoder;

// =====================================================================================================================
Decoder::Decoder(size_t bufferSize, Fifo& fifo, Controller& ctrl)
    : m_bufferSize(bufferSize),
      m_fifo(fifo),
      m_ctrl(ctrl)
{
}

// =====================================================================================================================
void Decoder::addFilter(const std::shared_ptr<filter::BaseFilter>& filter)
{
    m_filters.push_back(filter);
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
    bool working = false;

    while (1)
    {
	m_mutex.lock();

	// wait until we have some command to process
	while (m_commands.empty())
	    m_cond.wait(m_mutex);

	while (!m_commands.empty())
	{
	    std::shared_ptr<CmdBase> cmd = m_commands.front();
	    m_commands.pop_front();

	    std::cout << "decoder: cmd=" << cmd->m_cmd << std::endl;

	    switch (cmd->m_cmd)
	    {
		case INPUT :
		    m_input = static_cast<Input&>(*cmd).m_input;
		    break;

		case START :
		    if (!m_input)
		    {
			std::cerr << "decoder: unable to start working without input!" << std::endl;
			break;
		    }

		    working = true;

		    break;

		case STOP :
		    m_fifo.reset();
		    working = false;
		    break;

		case NOTIFY :
		    // do nothing here, this command is sent to wake up the decoder
		    break;
	    }
	}

	m_mutex.unlock();

	// do nothing if we are not working
	if (!working || !m_input)
	    continue;

	size_t fifoSize = m_fifo.getBytes();

	if (fifoSize >= m_bufferSize)
	    continue;

	// calculate the minimum size of the data we must put into the buffer
	size_t minSize = m_bufferSize - fifoSize;

	while (1)
	{
	    int16_t* samples;
	    size_t count;

	    if (!m_input->decode(samples, count))
	    {
		// the end of the stream has been reached
		std::cout << "decoder: end of stream" << std::endl;
		m_fifo.addMarker();

		// remove the input of the decoder
		m_input.reset();

		// let the controller know that the decoder finished working
		m_ctrl.command(Controller::DECODER_FINISHED);

		break;
	    }

	    // perform filters on the decoded samples
	    runFilters(samples, count);

	    size_t size = count * sizeof(int16_t) * m_input->getChannels();

	    m_fifo.addSamples(samples, size);

	    if (size < minSize)
		minSize -= size;
	    else
		break;
	}
    }
}

// =====================================================================================================================
void Decoder::runFilters(int16_t* samples, size_t count)
{
    float s[count * 2];

    // convert sample values to float
    for (size_t i = 0; i < count * 2; ++i)
	s[i] = (float)samples[i];

    for (const auto& filter : m_filters)
	filter->run(s, count);

    // convert the float samples back to the original buffer
    for (size_t i = 0; i < count * 2; ++i)
	samples[i] = (int16_t)s[i];
}
