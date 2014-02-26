/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef UTILS_SIGNALHANDLER_H_INCLUDED
#define UTILS_SIGNALHANDLER_H_INCLUDED

#include <signal.h>

namespace utils
{

class SignalHandler
{
    public:
	SignalHandler();

	void run();

    private:
	sigset_t m_set;
};

}

#endif
