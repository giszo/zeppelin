#ifndef PLAYER_PLAYBACKFIFO_H_INCLUDED
#define PLAYER_PLAYBACKFIFO_H_INCLUDED

#include <thread/mutex.h>

#include <deque>
#include <memory>

#include <stdint.h>

namespace player
{

class Fifo
{
    public:
	enum Type { NONE, SAMPLES, MARKER };

	Fifo(size_t bufferSize);

	void addSamples(const void* buffer, size_t size);
	void addMarker();

	size_t getBytes() const;
	Type getNextEvent();

	size_t readSamples(void* buffer, size_t size);

	void reset();

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

	thread::Mutex m_mutex;
};

}

#endif
