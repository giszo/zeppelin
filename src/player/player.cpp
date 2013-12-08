#include "player.h"

#include <output/alsa.h>
#include <codec/mp3.h>
#include <thread/blocklock.h>

#include <iostream>

#include <time.h>

using player::Player;

// =====================================================================================================================
Player::Player(buffer::RingBuffer& buffer)
    : m_buffer(buffer)
{
}

// =====================================================================================================================
void Player::setOutput(const std::shared_ptr<output::BaseOutput>& output)
{
    thread::BlockLock bl(m_mutex);
    m_output = output;
}

// =====================================================================================================================
void Player::play()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(PLAY);
}

// =====================================================================================================================
void Player::stop()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(STOP);
}

// =====================================================================================================================
void Player::run()
{
    bool playing = false;
    uint8_t buffer[2 /* channels */ * sizeof(int16_t) /* sample size */ * 10000 /* maximum samples per round */];

    std::shared_ptr<output::BaseOutput> output;

    while (1)
    {
	m_mutex.lock();

	while (!m_commands.empty())
	{
	    Command cmd = m_commands.front();
	    m_commands.pop_front();

	    std::cout << "player: cmd=" << cmd << std::endl;

	    switch (cmd)
	    {
		case PLAY :
		    if (playing)
			break;

		    if (!m_output)
		    {
			std::cerr << "player: no output to play on!" << std::endl;
			break;
		    }

		    // start the output and begin playing ...
		    m_output->start();
		    playing = true;

		    break;

		case STOP :
		    if (!playing)
			break;

		    if (!m_output)
		    {
			std::cerr << "player: no output to stop playing on!" << std::endl;
			break;
		    }

		    // stop the output
		    m_output->stop();
		    playing = false;

		    break;
	    }
	}

	if (playing)
	    output = m_output;

	m_mutex.unlock();

	if (!output)
	{
	    thread::Thread::sleep(100 * 1000);
	    continue;
	}

	// find out the available space on the output device for samples
	int availOutput = m_output->getAvailableSize() * sizeof(int16_t) * output->getChannels();
	int availInput = m_buffer.getAvailableSize();

	int size = std::min(availInput, availOutput);
	size = std::min(size, (int)sizeof(buffer));

	if (size > 0)
	{
	    // TODO: handle return value!
	    m_buffer.read(buffer, size);
	    m_output->write((int16_t*)buffer, size / (output->getChannels() * sizeof(int16_t)));
	}

	if (size == 0 || size == availOutput)
	    Thread::sleep(100 * 1000);
    }
}
