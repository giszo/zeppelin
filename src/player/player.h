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

class ControllerImpl;

class Player : public thread::Thread
{
    public:
	Player(const std::shared_ptr<output::BaseOutput>& output,
	       Fifo& fifo,
	       filter::Volume& volFilter,
	       ControllerImpl& ctrl);

	unsigned getPosition() const;

	void startPlayback();
	void pausePlayback();
	void stopPlayback();

	void seek(off_t seconds);

	void run();

    private:
	void processCommands();

    private:
	enum Command
	{
	    START,
	    PAUSE,
	    STOP,
	    SEEK
	};

	struct CmdBase
	{
	    CmdBase(Command cmd) : m_cmd(cmd) {}
	    Command m_cmd;
	};

	struct Seek : public CmdBase
	{
	    Seek(off_t seconds) : CmdBase(SEEK), m_seconds(seconds) {}
	    off_t m_seconds;
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

	std::deque<std::shared_ptr<CmdBase>> m_commands;

	thread::Mutex m_mutex;
	thread::Condition m_cond;
	thread::Condition m_emptyCond;

	// true when the player is currently working
	bool m_running;

	filter::Volume& m_volumeFilter;

	ControllerImpl& m_ctrl;
};

}

#endif
