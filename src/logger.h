#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <thread/mutex.h>

#include <iostream>

#define LOG(x) \
    Logger::get() << Logger::Lock() << x << "\n" << Logger::Unlock()

class Logger
{
    public:
	static Logger& get();

	template<typename T>
	Logger& operator<<(const T& t)
	{
	    std::cout << t;
	    return *this;
	}

	struct Lock {};
	struct Unlock {};

	Logger& operator<<(const Lock&);
	Logger& operator<<(const Unlock&);

    private:
	thread::Mutex m_mutex;

	static Logger s_logger;
};

#endif
