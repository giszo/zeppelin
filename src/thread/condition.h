/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef THREAD_CONDITION_H_INCLUDED
#define THREAD_CONDITION_H_INCLUDED

#include "mutex.h"

namespace thread
{

class Condition
{
    public:
	Condition();
	~Condition();

	void wait(Mutex& mutex);
	/**
	 * Waits for the given condition until the specified timeout.
	 * @param timeOut the timeout in microseconds
	 */
	void timedWait(Mutex& mutex, int timeOut);

	void signal();

    private:
	pthread_cond_t m_cond;
};

}

#endif
