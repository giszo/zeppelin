/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "thread.h"

#include <time.h>

using thread::Thread;

// =====================================================================================================================
void Thread::start()
{
    pthread_create(&m_thread, NULL, Thread::_starter, reinterpret_cast<void*>(this));
}

// =====================================================================================================================
void Thread::sleep(uint64_t usecs)
{
    timespec t;
    t.tv_sec = usecs / 1000000;
    t.tv_nsec = (usecs % 1000000) * 1000;
    nanosleep(&t, NULL);
}

// =====================================================================================================================
void* Thread::_starter(void* p)
{
    Thread* t = reinterpret_cast<Thread*>(p);
    t->run();
    return NULL;
}
