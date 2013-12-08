#ifndef PLAYER_PLAYER_H_INCLUDED
#define PLAYER_PLAYER_H_INCLUDED

#include <output/baseoutput.h>
#include <buffer/ringbuffer.h>
#include <thread/thread.h>

#include <deque>
#include <memory>

namespace player
{

class Player : public thread::Thread
{
    public:
	Player(buffer::RingBuffer& buffer);

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

	std::deque<Command> m_commands;

	buffer::RingBuffer& m_buffer;

	std::shared_ptr<output::BaseOutput> m_output;

	thread::Mutex m_mutex;
};

}

#endif
