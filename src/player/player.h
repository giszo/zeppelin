#ifndef PLAYER_PLAYER_H_INCLUDED
#define PLAYER_PLAYER_H_INCLUDED

#include "fifo.h"

#include <output/baseoutput.h>
#include <thread/thread.h>
#include <thread/condition.h>

#include <deque>
#include <memory>
#include <atomic>

namespace filter
{
class Volume;
}

namespace player
{

class Controller;

class Player : public thread::Thread
{
    public:
	Player(const std::shared_ptr<output::BaseOutput>& output,
	       Fifo& fifo,
	       filter::Volume& volFilter,
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

	// local buffer to store samples read from the fifo before playing them
	std::vector<float> m_buffer;

	// the sample buffer we are going to play from
	Fifo& m_fifo;

	// the output device instance
	std::shared_ptr<output::BaseOutput> m_output;

	// the format of the output device
	Format m_format;

	// number of played samples
	std::atomic_uint m_position;

	std::deque<Command> m_commands;

	thread::Mutex m_mutex;
	thread::Condition m_cond;
	thread::Condition m_emptyCond;

	// true when the player is currently working
	bool m_running;

	filter::Volume& m_volumeFilter;

	Controller& m_ctrl;
};

}

#endif
