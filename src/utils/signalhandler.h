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
