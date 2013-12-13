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
}

// =====================================================================================================================
void Player::pausePlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(PAUSE);
}

// =====================================================================================================================
void Player::stopPlayback()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(STOP);
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
	{
	    Thread::sleep(100 * 1000);
	    continue;
	}

	// find out the available space on the output device for samples
	size_t availOutput = m_output->getFreeSize() * sizeof(int16_t) * m_output->getChannels();

	// try to flush the fifo until it is empty ...
	auto event = m_fifo.getNextEvent();

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

	// now we can wait for a little because we put as much data into the output buffer as possible
	Thread::sleep(100 * 1000);
    }
}

// =====================================================================================================================
void Player::processCommands()
{
    thread::BlockLock bl(m_mutex);

    while (!m_commands.empty())
    {
	Command cmd = m_commands.front();
	m_commands.pop_front();

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
