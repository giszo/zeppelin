#ifndef PLAYER_QUEUE_H_INCLUDED
#define PLAYER_QUEUE_H_INCLUDED

#include "../library/musiclibrary.h"

#include <deque>

#include <pthread.h>

namespace player
{

class Queue
{
    public:
	Queue();
	~Queue();

	void add(const library::File& file);

	library::File pop();

    private:
	std::deque<library::File> m_queue;

	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;
};

}

#endif
