#include "player.h"

#include "../output/alsa.h"
#include "../codec/mp3.h"

#include <iostream>

using player::Player;

// =====================================================================================================================
Player::Player()
{
    m_output = std::make_shared<output::AlsaOutput>();
    m_output->setup(44100, 2);
}

// =====================================================================================================================
void Player::queue(const library::File& file)
{
    m_queue.add(file);
}

// =====================================================================================================================
void Player::play()
{
}

// =====================================================================================================================
void Player::stop()
{
}

// =====================================================================================================================
void Player::run()
{
    while (1)
    {
	// get a file to play
	library::File file = m_queue.pop();

	std::cout << "Playing: " << file.m_path << "/" << file.m_name << std::endl;

	codec::Mp3 mp3;
	mp3.open(file.m_path + "/" + file.m_name);

	if (mp3.getRate() != m_output->getRate() ||
	    mp3.getChannels() != m_output->getChannels())
	{
	    std::cerr << "format mismatch, skipping ..." << std::endl;
	    continue;
	}

	int16_t* samples;
	size_t count;

	// play until the input has available samples ...
	while (mp3.decode(samples, count))
	    m_output->write(samples, count);
    }
}
