#ifndef BUFFER_RINGBUFFER_H_INCLUDED
#define BUFFER_RINGBUFFER_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include <thread/mutex.h>

namespace buffer
{

class RingBuffer
{
    public:
	RingBuffer(size_t size);
	~RingBuffer();

	// returns the number of free bytes in the buffer
	size_t getFreeSize() const;
	// returns the size of the data inside the buffer
	size_t getAvailableSize() const;

	size_t read(void* data, size_t size);
	size_t write(const void* data, size_t size);

    private:
	// the storage for the buffer
	uint8_t* m_buffer;
	// the size of the buffer
	size_t m_size;

	// read pointer
	size_t m_readPos;
	// write pointer
	size_t m_writePos;

	// the number of items in the buffer
	size_t m_count;

	thread::Mutex m_mutex;
};

}

#endif
