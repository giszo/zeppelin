#include "decoder.h"
#include "controller.h"

#include <thread/blocklock.h>

#include <iostream>

using player::Decoder;

// =====================================================================================================================
Decoder::Decoder(buffer::RingBuffer& buffer, Controller& ctrl)
    : m_buffer(buffer),
      m_ctrl(ctrl)
{
}

// =====================================================================================================================
void Decoder::setInput(std::shared_ptr<codec::BaseCodec>& input)
{
    thread::BlockLock bl(m_mutex);
    m_input = input;
}

// =====================================================================================================================
void Decoder::work()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(WORK);
}

// =====================================================================================================================
void Decoder::suspend()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(SUSPEND);
}

// =====================================================================================================================
void Decoder::run()
{
    // TODO: this magic number should be a parameter of the decoder
    size_t minBufSize = 2 * sizeof(int16_t) * 44100;

    bool working = false;
    bool notified = false;

    while (1)
    {
	std::shared_ptr<codec::BaseCodec> input;

	m_mutex.lock();

	while (!m_commands.empty())
	{
	    Command cmd = m_commands.front();
	    m_commands.pop_front();

	    std::cout << "decoder: cmd=" << cmd << std::endl;

	    switch (cmd)
	    {
		case WORK :
		    if (!m_input)
		    {
			std::cerr << "decoder: unable to start working without input!" << std::endl;
			break;
		    }

		    working = true;
		    notified = false;

		    break;

		case SUSPEND :
		    working = false;
		    break;
	    }
	}

	if (working)
	    input = m_input;

	m_mutex.unlock();

	// do nothing if we are not working
	if (!input)
	{
	    thread::Thread::sleep(100 * 1000);
	    continue;
	}

	// calculate the minimum size of the data we must put into the buffer
	size_t minSize;

	if (m_buffer.getAvailableSize() < minBufSize)
	    minSize = minBufSize - m_buffer.getAvailableSize();
	else
	{
	    // the buffer is fine for now, do nothing ...
	    thread::Thread::sleep(100 * 1000);
	    continue;
	}

	while (1)
	{
	    int16_t* samples;
	    size_t count;

	    if (!input->decode(samples, count))
	    {
		// the end of the stream has been reached

		std::cout << "decoder: end of stream" << std::endl;

		thread::BlockLock bl(m_mutex);
		m_input.reset();

		break;
	    }

	    size_t size = count * sizeof(int16_t) * input->getChannels();

	    // TODO: handle the return value of write
	    m_buffer.write(reinterpret_cast<void*>(samples), size);

	    if (!notified)
	    {
		m_ctrl.command(Controller::DECODER_WORKING);
		notified = true;
	    }

	    if (size < minSize)
		minSize -= size;
	    else
		break;
	}
    }
}
