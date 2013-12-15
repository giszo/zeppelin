#ifndef PLAYER_DECODER_H_INCLUDED
#define PLAYER_DECODER_H_INCLUDED

#include "fifo.h"

#include <thread/thread.h>
#include <thread/mutex.h>
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
	Decoder(Fifo& fifo, Controller& ctrl);

	void addFilter(const std::shared_ptr<filter::BaseFilter>& filter);

	void setInput(const std::shared_ptr<codec::BaseCodec>& input);

	void startDecoding();
	void stopDecoding();

    private:
	void run() override;

	void runFilters(int16_t* samples, size_t count);

    private:
	enum Command
	{
	    START,
	    STOP
	};

	std::deque<Command> m_commands;

	/// samples will be put into this container
	Fifo& m_fifo;

	std::shared_ptr<codec::BaseCodec> m_input;

	/// filter chain that will be executed in the decoded samples
	std::vector<std::shared_ptr<filter::BaseFilter>> m_filters;

	thread::Mutex m_mutex;

	Controller& m_ctrl;
};

}

#endif
