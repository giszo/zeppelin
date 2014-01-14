#include "player.h"
#include "controller.h"

#include <output/alsa.h>
#include <codec/mp3.h>
#include <filter/volume.h>
#include <thread/blocklock.h>
#include <logger.h>

using player::Player;

// =====================================================================================================================
Player::Player(const std::shared_ptr<output::BaseOutput>& output,
	       Fifo& fifo,
	       filter::Volume& volFilter,
	       Controller& ctrl)
    : m_fifo(fifo),
      m_output(output),
      m_format(output->getFormat()),
      m_position(0),
      m_running(false),
      m_volumeFilter(volFilter),
      m_ctrl(ctrl)
{
}

// =====================================================================================================================
unsigned Player::getPosition() const
{
    return m_position / m_format.getRate();
}

// =====================================================================================================================
void Player::startPlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(START);
    m_cond.signal();
}

// =====================================================================================================================
void Player::pausePlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(PAUSE);
    m_cond.signal();
}

// =====================================================================================================================
void Player::stopPlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(STOP);
    m_cond.signal();

    // wait until all of the commands are processed, so the caller can make sure that playing is really stopped
    while (!m_commands.empty())
	m_emptyCond.wait(m_mutex);
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
		    m_position = 0;
		    m_ctrl.command(Controller::SONG_FINISHED);
		    break;

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
	Command cmd = m_commands.front();
	m_commands.pop_front();

	switch (cmd)
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
	}
    }

    m_emptyCond.signal();
}
