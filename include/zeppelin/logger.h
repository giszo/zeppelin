#ifndef ZEPPELIN_LOGGER_H_INCLUDED
#define ZEPPELIN_LOGGER_H_INCLUDED

#include <iostream>

#define LOG(x) \
    Logger::get() << Logger::Lock() << "[" << Logger::Timestamp() << "] " << x << "\n" << Logger::Unlock()

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
	struct Timestamp {};

	Logger& operator<<(const Lock&);
	Logger& operator<<(const Unlock&);
	Logger& operator<<(const Timestamp&);

    private:
	static Logger s_logger;
};

#endif
