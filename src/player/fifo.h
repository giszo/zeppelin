/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef PLAYER_PLAYBACKFIFO_H_INCLUDED
#define PLAYER_PLAYBACKFIFO_H_INCLUDED

#include <thread/mutex.h>

#include <deque>
#include <memory>
#include <functional>

#include <stdint.h>

namespace player
{

class Fifo
{
    public:
	enum Type { NONE, SAMPLES, MARKER };

	typedef std::function<void ()> NotifyCallback;

	Fifo(size_t bufferSize);

	void addSamples(const void* buffer, size_t size);
	void addMarker();

	size_t getBytes() const;
	Type getNextEvent();

	size_t readSamples(void* buffer, size_t size);

	void reset();

	void setNotifyCallback(size_t mark, const NotifyCallback& cb);

    private:
	struct Item
	{
	    Item(Type type) : m_type(type) {}
	    virtual ~Item() {}
	    Type m_type;
	};

	struct Samples : public Item
	{
	    Samples(size_t size) : Item(SAMPLES), m_data(new uint8_t[size]), m_size(size) {}
	    ~Samples() { delete[] m_data; }

	    uint8_t* m_data;
	    size_t m_size;
	    size_t m_realSize;
	    size_t m_offset;
	};

	/// the actual playback fifo
	std::deque<std::shared_ptr<Item>> m_fifo;

	/// queue for reusable Samples objects
	std::deque<std::shared_ptr<Samples>> m_samples;

	/// returns the number of bytes in the playback fifo
	size_t m_bytesInFifo;

	/// size used for allocating new buffers to store samples
	size_t m_bufferSize;

	// if set to non-zero notify callback will be called once the number of bytes in the fifo goes below this limit
	size_t m_notifySize;
	NotifyCallback m_notifyCb;

	thread::Mutex m_mutex;
};

}

#endif
