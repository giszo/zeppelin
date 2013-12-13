#ifndef PLAYER_DECODER_H_INCLUDED
#define PLAYER_DECODER_H_INCLUDED

#include "fifo.h"

#include <thread/thread.h>
#include <thread/mutex.h>
#include <codec/basecodec.h>

#include <memory>
#include <deque>

namespace player
{

class Controller;

class Decoder : public thread::Thread
{
    public:
	Decoder(Fifo& fifo, Controller& ctrl);

	void setInput(const std::shared_ptr<codec::BaseCodec>& input);

	void startDecoding();
	void stopDecoding();

    private:
	void run() override;

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

	thread::Mutex m_mutex;

	Controller& m_ctrl;
};

}

#endif
