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
	/// availables commands for the controller
	enum Command
	{
	    PLAY,
	    STOP,
	    // sent by the player thread once the sample buffer is empty
	    PLAYER_DONE,
	    // sent by the decoder once it started to fill the sample buffer with data
	    DECODER_WORKING
	};

	Controller();

	/// returns the current play queue
	std::vector<std::shared_ptr<library::File>> getQueue();

	/// puts a new file onto the playback queue
	void queue(const std::shared_ptr<library::File>& file);

	void play();
	void stop();

	void command(Command cmd);

	/// the mainloop of the controller
	void run();

    private:
	void startPlayback();

    private:
	/// queued files for playing
	std::deque<std::shared_ptr<library::File>> m_queue;

	/// the currently played file
	std::shared_ptr<library::File> m_currentFile;

	/// the codec used to decode the input file
	std::shared_ptr<codec::BaseCodec> m_input;

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
