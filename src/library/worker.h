#ifndef LIBRARY_WORKER_H_INCLUDED
#define LIBRARY_WORKER_H_INCLUDED

#include <deque>
#include <memory>

#include <pthread.h>

namespace library
{

class BaseWork
{
    public:
	virtual ~BaseWork()
	{}

	virtual void run() = 0;
};

class Worker
{
    public:
	Worker();

	void add(const std::shared_ptr<BaseWork>& work);

	void start();

    private:
	void run();

	static void* _run(void* p);

    private:
	std::deque<std::shared_ptr<BaseWork>> m_queue;

	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;

	pthread_t m_thread;
};

}

#endif
