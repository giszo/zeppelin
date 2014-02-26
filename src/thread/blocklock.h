/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef THREAD_BLOCKLOCK_H_INCLUDED
#define THREAD_BLOCKLOCK_H_INCLUDED

#include "mutex.h"

namespace thread
{

class BlockLock
{
    public:
	BlockLock(const Mutex& mutex)
	    : m_mutex(mutex)
	{ m_mutex.lock(); }
	~BlockLock()
	{ m_mutex.unlock(); }

    private:
	const Mutex& m_mutex;
};

}

#endif
