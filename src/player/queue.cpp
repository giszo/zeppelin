#include "queue.h"

using player::Queue;

// =====================================================================================================================
Queue::Queue()
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
}

// =====================================================================================================================
Queue::~Queue()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_lock);
}

// =====================================================================================================================
void Queue::add(const library::File& file)
{
    pthread_mutex_lock(&m_lock);
    m_queue.push_back(file);
    pthread_mutex_unlock(&m_lock);
    pthread_cond_signal(&m_cond);
}

// =====================================================================================================================
library::File Queue::pop()
{
    library::File file;

    pthread_mutex_lock(&m_lock);

    while (m_queue.empty())
	pthread_cond_wait(&m_cond, &m_lock);

    file = m_queue.front();
    m_queue.pop_front();

    pthread_mutex_unlock(&m_lock);

    return file;
}
