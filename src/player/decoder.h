#ifndef PLAYER_DECODER_H_INCLUDED
#define PLAYER_DECODER_H_INCLUDED

#include "fifo.h"

#include <thread/thread.h>
#include <thread/mutex.h>
#include <thread/condition.h>
#include <codec/basecodec.h>
#include <filter/basefilter.h>

#include <memory>
#include <deque>
#include <vector>

namespace player
{

class Controller;

class Decoder : public thread::Thread
{
    public:
	Decoder(size_t bufferSize, Fifo& fifo, Controller& ctrl);

	void addFilter(const std::shared_ptr<filter::BaseFilter>& filter);

	void setInput(const std::shared_ptr<codec::BaseCodec>& input);

	void startDecoding();
	void stopDecoding();

	void notify();

    private:
	void run() override;

	void runFilters(float* samples, size_t count, const Format& format);

    private:
	enum Command
	{
	    INPUT,
	    START,
	    STOP,
	    NOTIFY
	};

	struct CmdBase
	{
	    CmdBase(Command cmd) : m_cmd(cmd) {}
	    Command m_cmd;
	};

	struct Input : public CmdBase
	{
	    Input(const std::shared_ptr<codec::BaseCodec>& input) : CmdBase(INPUT), m_input(input) {}
	    std::shared_ptr<codec::BaseCodec> m_input;
	};

	std::deque<std::shared_ptr<CmdBase>> m_commands;

	// the minimum size of the buffer until the decoder should fill it
	size_t m_bufferSize;

	/// samples will be put into this container
	Fifo& m_fifo;

	std::shared_ptr<codec::BaseCodec> m_input;

	// format of the current input
	Format m_format;

	/// filter chain that will be executed in the decoded samples
	std::vector<std::shared_ptr<filter::BaseFilter>> m_filters;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	Controller& m_ctrl;
};

}

#endif
