#include "player.h"
#include "controller.h"

#include <output/alsa.h>
#include <codec/mp3.h>
#include <thread/blocklock.h>

#include <iostream>

using player::Player;

// =====================================================================================================================
Player::Player(buffer::RingBuffer& buffer, Controller& ctrl)
    : m_playing(false),
      m_buffer(buffer),
      m_ctrl(ctrl)
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
    uint8_t buffer[2 /* channels */ * sizeof(int16_t) /* sample size */ * 10000 /* maximum samples per round */];

    while (1)
    {
	std::shared_ptr<output::BaseOutput> output;

	m_mutex.lock();

	// process queued commands
	while (!m_commands.empty())
	{
	    Command cmd = m_commands.front();
	    m_commands.pop_front();

	    processCommand(cmd);
	}

	// copy the output device pointer while holding the lock
	if (m_playing)
	    output = m_output;

	m_mutex.unlock();

	// do nothing if we have no output device, it means we are not playing
	if (!output)
	{
	    thread::Thread::sleep(100 * 1000);
	    continue;
	}

	// fid out how many samples we have to play
	int availInput = m_buffer.getAvailableSize();

	// end of the current playback session
	if (availInput == 0)
	{
	    // tell the controller that the player has finished its work
	    m_ctrl.command(Controller::PLAYER_DONE);
	    m_playing = false;
	    continue;
	}

	// find out the available space on the output device for samples
	int availOutput = m_output->getAvailableSize() * sizeof(int16_t) * output->getChannels();

	int size = std::min(availInput, availOutput);
	size = std::min(size, (int)sizeof(buffer));

	if (size > 0)
	{
	    // TODO: handle return value!
	    m_buffer.read(buffer, size);
	    m_output->write((int16_t*)buffer, size / (output->getChannels() * sizeof(int16_t)));
	}

	// sleep if we had nothing to play this round or we filled the whole space in the output buffer
	if (size == 0 || size == availOutput)
	    Thread::sleep(100 * 1000);
    }
}

// =====================================================================================================================
void Player::processCommand(Command cmd)
{
    std::cout << "player: cmd=" << cmd << std::endl;

    switch (cmd)
    {
	case PLAY :
	    if (m_playing)
		break;

	    if (!m_output)
	    {
		std::cerr << "player: no output to play on!" << std::endl;
		break;
	    }

	    // start the output and begin playing ...
	    m_output->start();
	    m_playing = true;

	    break;

	case STOP :
	    if (!m_playing)
		break;

	    if (!m_output)
	    {
		std::cerr << "player: no output to stop playing on!" << std::endl;
		break;
	    }

	    // stop the output
	    m_output->stop();
	    m_playing = false;

	    break;
    }
}
