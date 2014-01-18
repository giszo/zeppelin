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

class ControllerImpl;

class Decoder : public thread::Thread
{
    public:
	Decoder(size_t bufferSize,
		const Format& outputFormat,
		Fifo& fifo,
		ControllerImpl& ctrl,
		const config::Config& config);

	void setInput(const std::shared_ptr<codec::BaseCodec>& input);

	void startDecoding();
	void stopDecoding();

	void seek(off_t seconds);
	void notify();

    private:
	void run() override;

	void runFilters(float*& samples, size_t& count, const Format& format);

	void turnOnResampling();

    private:
	enum Command
	{
	    INPUT,
	    START,
	    STOP,
	    SEEK,
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

	struct Seek : public CmdBase
	{
	    Seek(off_t seconds) : CmdBase(SEEK), m_seconds(seconds) {}
	    off_t m_seconds;
	};

	std::deque<std::shared_ptr<CmdBase>> m_commands;

	// the minimum size of the buffer until the decoder should fill it
	size_t m_bufferSize;

	/// samples will be put into this container
	Fifo& m_fifo;

	std::shared_ptr<codec::BaseCodec> m_input;

	// format of the current input
	Format m_format;
	// format of the output device
	Format m_outputFormat;

	// true when resampling is turned on because input and output sampling rate differs
	bool m_resampling;

	/// filter chain that will be executed in the decoded samples
	std::vector<std::shared_ptr<filter::BaseFilter>> m_filters;

	thread::Mutex m_mutex;
	thread::Condition m_cond;
	thread::Condition m_emptyCond;

	ControllerImpl& m_ctrl;
	const config::Config& m_config;
};

}

#endif
