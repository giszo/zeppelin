#ifndef PLAYER_PLAYER_H_INCLUDED
#define PLAYER_PLAYER_H_INCLUDED

#include "fifo.h"

#include <output/baseoutput.h>
#include <thread/thread.h>
#include <thread/condition.h>

#include <deque>
#include <memory>
#include <atomic>

namespace player
{

class Controller;

class Player : public thread::Thread
{
    public:
	Player(const std::shared_ptr<output::BaseOutput>& output,
	       Fifo& fifo,
	       Controller& ctrl);

	unsigned getPosition() const;

	void startPlayback();
	void pausePlayback();
	void stopPlayback();

	void run();

    private:
	void processCommands();

    private:
	enum Command
	{
	    START,
	    PAUSE,
	    STOP
	};

	// the sample buffer we are going to play from
	Fifo& m_fifo;

	// the output device instance
	std::shared_ptr<output::BaseOutput> m_output;

	// number of played samples
	std::atomic_uint m_position;

	std::deque<Command> m_commands;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	// true when the player is currently working
	bool m_running;

	Controller& m_ctrl;
};

}

#endif
