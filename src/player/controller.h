#ifndef PLAYER_CONTORLLER_H_INCLUDED
#define PLAYER_CONTORLLER_H_INCLUDED

#include "decoder.h"
#include "player.h"

#include <buffer/ringbuffer.h>
#include <library/musiclibrary.h>
#include <thread/condition.h>

#include <vector>

namespace player
{

class Controller
{
    public:
	Controller();

	/// returns the current play queue
	std::vector<std::shared_ptr<library::File>> getQueue();

	/// puts a new file onto the playback queue
	void queue(const std::shared_ptr<library::File>& file);

	void play();
	void stop();

	void run();

    private:
	/// queued files for playing
	std::vector<std::shared_ptr<library::File>> m_queue;

	enum Command
	{
	    PLAY,
	    STOP
	};

	/// controller commands
	std::deque<Command> m_commands;

	/// buffer for storing decoded samples
	buffer::RingBuffer m_samples;

	/// input decoder thread filling the sample buffer
	Decoder m_decoder;
	/// player thread putting decoded samples to the output device
	Player m_player;

	thread::Mutex m_mutex;
	thread::Condition m_cond;
};

}

#endif
