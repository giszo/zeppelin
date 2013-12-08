#ifndef PLAYER_PLAYER_H_INCLUDED
#define PLAYER_PLAYER_H_INCLUDED

#include <output/baseoutput.h>
#include <buffer/ringbuffer.h>
#include <thread/thread.h>

#include <deque>
#include <memory>

namespace player
{

class Controller;

class Player : public thread::Thread
{
    public:
	Player(buffer::RingBuffer& buffer, Controller& ctrl);

	void setOutput(const std::shared_ptr<output::BaseOutput>& output);

	void play();
	void stop();

	void run();

    private:
	enum Command
	{
	    PLAY,
	    STOP
	};

	void processCommand(Command cmd);

    private:
	// true when the player is running
	bool m_playing;

	// queue of player commands
	std::deque<Command> m_commands;

	// the sample buffer we are going to play from
	buffer::RingBuffer& m_buffer;

	// the output device instance
	std::shared_ptr<output::BaseOutput> m_output;

	thread::Mutex m_mutex;

	Controller& m_ctrl;
};

}

#endif
