#include "logger.h"

#include <sys/time.h>
#include <time.h>

Logger Logger::s_logger;

// =====================================================================================================================
Logger& Logger::get()
{
    return s_logger;
}

// =====================================================================================================================
Logger& Logger::operator<<(const Lock&)
{
    m_mutex.lock();
    return *this;
}

// =====================================================================================================================
Logger& Logger::operator<<(const Unlock&)
{
    m_mutex.unlock();
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

    std::cout << buf << "." << tv.tv_usec;

    return *this;
}
