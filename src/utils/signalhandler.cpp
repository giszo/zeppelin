#include "signalhandler.h"

#include <csignal>
#include <cstdlib>

using utils::SignalHandler;

// =====================================================================================================================
SignalHandler::SignalHandler()
{
    // mask interesting signals because this signal mask will be inherited by each thread we start later on
    sigemptyset(&m_set);
    sigaddset(&m_set, SIGHUP);
    sigaddset(&m_set, SIGINT);
    sigaddset(&m_set, SIGTERM);
    sigaddset(&m_set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &m_set, NULL);
}

// =====================================================================================================================
void SignalHandler::run()
{
    bool running = true;

    while (running)
    {
	int number;

	// wait for a signal
	sigwait(&m_set, &number);

	switch (number)
	{
	    case SIGINT :
	    case SIGTERM :
		// exit the application
		running = false;
		break;

	    case SIGHUP :
	    case SIGPIPE :
		// do nothing with these signals
		break;
	}
    }
}
