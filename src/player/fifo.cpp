/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "fifo.h"

#include <thread/blocklock.h>

#include <cstring>

using player::Fifo;

// =====================================================================================================================
Fifo::Fifo(size_t bufferSize)
    : m_bytesInFifo(0),
      m_bufferSize(bufferSize),
      m_notifySize(0)
{
}

// =====================================================================================================================
void Fifo::addSamples(const void* buffer, size_t size)
{
    const uint8_t* d = (const uint8_t*)buffer;

    thread::BlockLock bl(m_mutex);

    // TODO: try to append some data to the last buffer

    m_bytesInFifo += size;

    while (size > 0)
    {
	std::shared_ptr<Samples> buf;

	if (m_samples.empty())
	    buf = std::make_shared<Samples>(m_bufferSize);
	else
	{
	    buf = m_samples.front();
	    m_samples.pop_front();
	}

	buf->m_offset = 0;
	buf->m_realSize = std::min(buf->m_size, size);
	memcpy(buf->m_data, d, buf->m_realSize);

	m_fifo.push_back(buf);

	d += buf->m_realSize;
	size -= buf->m_realSize;
    }
}

// =====================================================================================================================
void Fifo::addMarker()
{
    thread::BlockLock bl(m_mutex);
    m_fifo.push_back(std::make_shared<Item>(MARKER));
}

// =====================================================================================================================
size_t Fifo::getBytes() const
{
    thread::BlockLock bl(m_mutex);
    return m_bytesInFifo;
}

// =====================================================================================================================
auto Fifo::getNextEvent() -> Type
{
    thread::BlockLock bl(m_mutex);

    if (m_fifo.empty())
	return NONE;

    auto event = m_fifo.front()->m_type;

    // remove the marker here from the fifo
    if (event == MARKER)
	m_fifo.pop_front();

    return event;
}

// =====================================================================================================================
size_t Fifo::readSamples(void* buffer, size_t size)
{
    thread::BlockLock bl(m_mutex);

    size_t r = 0;
    uint8_t* d = reinterpret_cast<uint8_t*>(buffer);

    while (!m_fifo.empty())
    {
	std::shared_ptr<Item> item = m_fifo.front();

	if (item->m_type != SAMPLES)
	    break;

	Samples& buf = reinterpret_cast<Samples&>(*item);
	size_t s = std::min(size, buf.m_realSize);

	memcpy(d, buf.m_data + buf.m_offset, s);

	d += s;
	r += s;
	size -= s;

	if (s == buf.m_realSize)
	{
	    m_fifo.pop_front();
	    m_samples.push_back(std::dynamic_pointer_cast<Samples>(item));
	}
	else
	{
	    buf.m_offset += s;
	    buf.m_realSize -= s;
	    break;
	}
    }

    m_bytesInFifo -= r;

    // check whether we need to notify someone ...
    if (m_notifySize > 0 && m_bytesInFifo < m_notifySize)
	m_notifyCb();

    return r;
}

// =====================================================================================================================
void Fifo::reset()
{
    thread::BlockLock bl(m_mutex);

    while (!m_fifo.empty())
    {
	std::shared_ptr<Item> item = m_fifo.front();

	if (item->m_type == SAMPLES)
	    m_samples.push_back(std::dynamic_pointer_cast<Samples>(item));

	m_fifo.pop_front();
    }

    m_bytesInFifo = 0;
}

// =====================================================================================================================
void Fifo::setNotifyCallback(size_t mark, const NotifyCallback& cb)
{
    m_notifySize = mark;
    m_notifyCb = cb;
}
