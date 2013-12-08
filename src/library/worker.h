#ifndef LIBRARY_WORKER_H_INCLUDED
#define LIBRARY_WORKER_H_INCLUDED

#include <thread/thread.h>

#include <deque>
#include <memory>

namespace library
{

class BaseWork
{
    public:
	virtual ~BaseWork()
	{}

	virtual void run() = 0;
};

class Worker : public thread::Thread
{
    public:
	Worker();

	void add(const std::shared_ptr<BaseWork>& work);

    private:
	void run() override;

    private:
	std::deque<std::shared_ptr<BaseWork>> m_queue;

	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;
};

}

#endif
