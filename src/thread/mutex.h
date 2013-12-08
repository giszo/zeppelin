#ifndef THREAD_MUTEX_H_INCLUDED
#define THREAD_MUTEX_H_INCLUDED

#include <pthread.h>

namespace thread
{

class Mutex
{
    public:
	Mutex()
	{ pthread_mutex_init(&m_mutex, NULL); }
	~Mutex()
	{ pthread_mutex_destroy(&m_mutex); }

	void lock() const
	{ pthread_mutex_lock(&m_mutex); }
	void unlock() const
	{ pthread_mutex_unlock(&m_mutex); }

	pthread_mutex_t* get()
	{ return &m_mutex; }

    private:
	mutable pthread_mutex_t m_mutex;
};

}

#endif
