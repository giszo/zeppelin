#ifndef THREAD_CONDITION_H_INCLUDED
#define THREAD_CONDITION_H_INCLUDED

#include "mutex.h"

namespace thread
{

class Condition
{
    public:
	Condition()
	{ pthread_cond_init(&m_cond, NULL); }
	~Condition()
	{ pthread_cond_destroy(&m_cond); }

	void wait(Mutex& mutex)
	{ pthread_cond_wait(&m_cond, mutex.get()); }
	void signal()
	{ pthread_cond_signal(&m_cond); }

    private:
	pthread_cond_t m_cond;
};

}

#endif
