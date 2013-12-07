#include "worker.h"

using library::Worker;

// =====================================================================================================================
Worker::Worker()
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
}

// =====================================================================================================================
void Worker::add(const std::shared_ptr<BaseWork>& work)
{
    pthread_mutex_lock(&m_lock);
    m_queue.push_back(work);
    pthread_mutex_unlock(&m_lock);
    pthread_cond_signal(&m_cond);
}

// =====================================================================================================================
void Worker::start()
{
    pthread_create(&m_thread, NULL, _run, reinterpret_cast<void*>(this));
}

// =====================================================================================================================
void Worker::run()
{
    while (1)
    {
	pthread_mutex_lock(&m_lock);

	// wait while we get some work ... :)
	while (m_queue.empty())
	    pthread_cond_wait(&m_cond, &m_lock);

	std::shared_ptr<BaseWork> work = m_queue.front();
	m_queue.pop_front();

	pthread_mutex_unlock(&m_lock);

	work->run();
    }
}

// =====================================================================================================================
void* Worker::_run(void* p)
{
    reinterpret_cast<Worker*>(p)->run();
    return NULL;
}
