#ifndef PLAYER_CONTORLLER_H_INCLUDED
#define PLAYER_CONTORLLER_H_INCLUDED

#include "decoder.h"
#include "player.h"

#include <buffer/ringbuffer.h>
#include <library/musiclibrary.h>
#include <thread/condition.h>

#include <deque>

namespace player
{

class Controller
{
    public:
	Controller();

	void queue(const library::File& file);

	void play();
	void stop();

	void run();

    private:
	std::deque<library::File> m_queue;

	enum Command
	{
	    PLAY,
	    STOP
	};

	std::deque<Command> m_commands;

	// buffer for storing decoded samples
	buffer::RingBuffer m_samples;

	Decoder m_decoder;
	Player m_player;

	thread::Mutex m_mutex;
	thread::Condition m_cond;
};

}

#endif
