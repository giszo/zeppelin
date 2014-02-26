/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "condition.h"

#include <ctime>

using thread::Condition;

// =====================================================================================================================
Condition::Condition()
{
    pthread_condattr_t attr;

    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

    pthread_cond_init(&m_cond, &attr);

    pthread_condattr_destroy(&attr);
}

// =====================================================================================================================
Condition::~Condition()
{
    pthread_cond_destroy(&m_cond);
}

// =====================================================================================================================
void Condition::wait(Mutex& mutex)
{
    pthread_cond_wait(&m_cond, mutex.get());
}

// =====================================================================================================================
void Condition::timedWait(Mutex& mutex, int timeOut)
{
    timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    ts.tv_sec += timeOut / 1000000;
    ts.tv_nsec += (timeOut % 1000000) * 1000;

    if (ts.tv_nsec >= 1000000000)
    {
	++ts.tv_sec;
	ts.tv_nsec -= 1000000000;
    }

    pthread_cond_timedwait(&m_cond, mutex.get(), &ts);
}

// =====================================================================================================================
void Condition::signal()
{
    pthread_cond_signal(&m_cond);
}
