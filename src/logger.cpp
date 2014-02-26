/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include <thread/mutex.h>

#include <zeppelin/logger.h>

#include <iomanip>

#include <sys/time.h>
#include <time.h>

Logger Logger::s_logger;

static thread::Mutex s_mutex;

// =====================================================================================================================
Logger& Logger::get()
{
    return s_logger;
}

// =====================================================================================================================
Logger& Logger::operator<<(const Lock&)
{
    s_mutex.lock();
    return *this;
}

// =====================================================================================================================
Logger& Logger::operator<<(const Unlock&)
{
    s_mutex.unlock();
    return *this;
}

// =====================================================================================================================
Logger& Logger::operator<<(const Timestamp&)
{
    timeval tv;
    gettimeofday(&tv, NULL);

    time_t t = tv.tv_sec;
    tm* tm =  localtime(&t);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);

    std::cout << buf << "." << std::setw(6) << std::setfill('0') << tv.tv_usec;

    return *this;
}
