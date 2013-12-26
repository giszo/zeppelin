#include "player.h"
#include "controller.h"

#include <output/alsa.h>
#include <codec/mp3.h>
#include <thread/blocklock.h>

#include <iostream>

using player::Player;

// =====================================================================================================================
Player::Player(const std::shared_ptr<output::BaseOutput>& output, Fifo& fifo, Controller& ctrl)
    : m_fifo(fifo),
      m_output(output),
      m_position(0),
      m_running(false),
      m_ctrl(ctrl)
{
}

// =====================================================================================================================
unsigned Player::getPosition() const
{
    return m_position / m_output->getRate();
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
}

// =====================================================================================================================
void Player::run()
{
    uint8_t buffer[2 /* channels */ * sizeof(int16_t) /* sample size */ * 10000 /* maximum samples per round */];

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
	size_t availOutput = m_output->getFreeSize() * sizeof(int16_t) * m_output->getChannels();

	// try to flush the fifo until it is empty ...
	while (event != Fifo::NONE)
	{
	    switch (event)
	    {
		case Fifo::SAMPLES :
		{
		    size_t size = std::min(availOutput, sizeof(buffer));
		    size_t res = m_fifo.readSamples(buffer, size);
		    size_t samples = res / (m_output->getChannels() * sizeof(int16_t));
		    m_output->write(reinterpret_cast<int16_t*>(buffer), samples);
		    m_position += samples;
		    availOutput -= res;
		    break;
		}

		case Fifo::MARKER :
		    m_position = 0;
		    m_ctrl.command(Controller::SONG_FINISHED);
		    break;

		case Fifo::NONE :
		    break;
	    }

	    if (availOutput == 0)
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

	std::cout << "player: cmd=" << cmd << std::endl;

	switch (cmd)
	{
	    case START :
		m_running = true;
		break;

	    case STOP :
		m_position = 0;
		// NOTE: there is no break here intentionally!

	    case PAUSE :
		m_running = false;
		m_output->drop();
		break;
	}
    }
}
