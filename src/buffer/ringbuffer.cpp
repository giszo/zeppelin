#include "ringbuffer.h"

#include <thread/blocklock.h>

#include <iostream>

using buffer::RingBuffer;

// TODO: read() and write() functions are far from being optimal ... :]

// =====================================================================================================================
RingBuffer::RingBuffer(size_t size)
    : m_buffer(new uint8_t[size]),
      m_size(size),
      m_readPos(0),
      m_writePos(0),
      m_count(0)
{
}

// =====================================================================================================================
RingBuffer::~RingBuffer()
{
    delete[] m_buffer;
}

// =====================================================================================================================
size_t RingBuffer::getFreeSize() const
{
    thread::BlockLock bl(m_mutex);
    return m_size - m_count;
}

// =====================================================================================================================
size_t RingBuffer::getAvailableSize() const
{
    thread::BlockLock bl(m_mutex);
    return m_count;
}

// =====================================================================================================================
size_t RingBuffer::read(void* data, size_t size)
{
    thread::BlockLock bl(m_mutex);

    uint8_t* d = (uint8_t*)data;
    size_t s = std::min(m_count, size);

    for (size_t i = 0; i < s; ++i)
    {
	d[i] = m_buffer[m_readPos++];
	m_readPos %= m_size;
    }

    m_count -= s;

    return s;
}

// =====================================================================================================================
size_t RingBuffer::write(const void* data, size_t size)
{
    thread::BlockLock bl(m_mutex);

    uint8_t* d = (uint8_t*)data;
    size_t s = std::min(m_size - m_count, size);

    for (size_t i = 0; i < s; ++i)
    {
	m_buffer[m_writePos++] = d[i];
	m_writePos %= m_size;
    }

    m_count += s;

    return s;
}
