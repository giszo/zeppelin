#include "player.h"
#include "controller.h"

#include <output/alsa.h>
#include <codec/mp3.h>
#include <filter/volume.h>
#include <thread/blocklock.h>

#include <zeppelin/logger.h>

using player::Player;

// =====================================================================================================================
Player::Player(const std::shared_ptr<output::BaseOutput>& output,
	       Fifo& fifo,
	       const config::Config& config)
    : m_fifo(fifo),
      m_output(output),
      m_format(output->getFormat()),
      m_position(0),
      m_running(false),
      m_volumeFilter(config)
{
}

// =====================================================================================================================
void Player::setController(const std::weak_ptr<ControllerImpl>& controller)
{
    m_ctrl = controller;
}

// =====================================================================================================================
unsigned Player::getPosition() const
{
    return m_position / m_format.getRate();
}

// =====================================================================================================================
filter::Volume& Player::getVolumeFilter()
{
    return m_volumeFilter;
}

// =====================================================================================================================
void Player::startPlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(START));
    m_cond.signal();
}

// =====================================================================================================================
void Player::pausePlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(PAUSE));
    m_cond.signal();
}

// =====================================================================================================================
void Player::stopPlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(STOP));
    m_cond.signal();

    // wait until all of the commands are processed, so the caller can make sure that playing is really stopped
    while (!m_commands.empty())
	m_emptyCond.wait(m_mutex);
}

// =====================================================================================================================
void Player::seek(off_t seconds)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<Seek>(seconds));
    m_cond.signal();
}

// =====================================================================================================================
void Player::run()
{
    while (1)
    {
	processCommands();

	// do nothing if the player is stopped
	if (!m_running)
	    continue;

	// get the next event from the fifo
	auto event = m_fifo.getNextEvent();

	if (event == Fifo::NONE)
	    continue;

	// find out the available space on the output device for samples
	int availSamples = m_output->getFreeSize();

	// try to flush the fifo until it is empty ...
	while (event != Fifo::NONE)
	{
	    switch (event)
	    {
		case Fifo::SAMPLES :
		{
		    // reserve enough space in the buffer
		    m_buffer.resize(availSamples * m_format.getChannels());

		    // read samples from the fifo
		    size_t res = m_fifo.readSamples(&m_buffer[0], m_format.sizeOfSamples(availSamples));
		    size_t samples = m_format.numOfSamples(res);

		    // perform volume filter
		    float* p = &m_buffer[0];
		    m_volumeFilter.run(p, samples, m_format);

		    // play the data
		    m_output->write(p, samples);

		    m_position += samples;
		    availSamples -= samples;

		    break;
		}

		case Fifo::MARKER :
		{
		    m_position = 0;

		    auto ctrl = m_ctrl.lock();

		    if (ctrl)
			ctrl->command(ControllerImpl::SONG_FINISHED);

		    break;
		}

		case Fifo::NONE :
		    // this should be never reached because NONE terminates the loop
		    break;
	    }

	    if (availSamples == 0)
		break;

	    event = m_fifo.getNextEvent();
	}
    }
}

// =====================================================================================================================
void Player::processCommands()
{
    thread::BlockLock bl(m_mutex);

    // wait until we get a command or a timeout (100ms)
    m_cond.timedWait(m_mutex, 100 * 1000);

    while (!m_commands.empty())
    {
	std::shared_ptr<CmdBase> cmd = m_commands.front();
	m_commands.pop_front();

	switch (cmd->m_cmd)
	{
	    case START :
		LOG("player: start");
		m_running = true;
		break;

	    case STOP :
		LOG("player: stop");
		m_position = 0;
		m_running = false;
		m_output->drop();
		break;

	    case PAUSE :
		LOG("player: pause");
		m_running = false;
		m_output->drop();
		break;

	    case SEEK :
	    {
		Seek& s = static_cast<Seek&>(*cmd);
		LOG("player: seek " << s.m_seconds);
		m_position = s.m_seconds * m_format.getRate();
		break;
	    }
	}
    }

    m_emptyCond.signal();
}
