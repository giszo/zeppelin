/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef THREAD_THREAD_H_INCLUDED
#define THREAD_THREAD_H_INCLUDED

#include <stdint.h>
#include <pthread.h>

namespace thread
{

class Thread
{
    public:
	virtual ~Thread()
	{}

	void start();

	virtual void run() = 0;

	/// sleeps for the given number of microseconds
	static void sleep(uint64_t usecs);

    private:
	static void* _starter(void* p);

	pthread_t m_thread;
};

}

#endif
