#ifndef PLAYER_DECODER_H_INCLUDED
#define PLAYER_DECODER_H_INCLUDED

#include <thread/thread.h>
#include <thread/mutex.h>
#include <buffer/ringbuffer.h>
#include <codec/basecodec.h>

#include <memory>
#include <deque>

namespace player
{

class Decoder : public thread::Thread
{
    public:
	Decoder(buffer::RingBuffer& buffer);

	void setInput(std::shared_ptr<codec::BaseCodec>& input);

	// notifies the decoder thread to start working
	void work();
	// notifies the decoder thread to suspend its job
	void suspend();

    private:
	void run() override;

    private:
	enum Command
	{
	    WORK,
	    SUSPEND
	};

	std::deque<Command> m_commands;

	buffer::RingBuffer& m_buffer;

	std::shared_ptr<codec::BaseCodec> m_input;

	thread::Mutex m_mutex;
};

}

#endif
