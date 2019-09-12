#include <signal.h>

#include <vector>

#include "Logger.h"
#include "Signal_Handler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Private

static sigset_t set_sigset(std::vector<int> const & signal_numbers)
{
    sigset_t signal_set {};

    if (sigemptyset(&signal_set) != 0)
    {
        Logger::log_and_abort("Signal_Handler", "sigemptyset", errno, __FILE__, __LINE__);
    }

    for (auto const & signal_number : signal_numbers)
    {
        if (sigaddset(&signal_set, signal_number) != 0)
        {
            Logger::log_and_abort("Signal_Handler", "sigaddset", errno, __FILE__, __LINE__);
        }
    }

    return signal_set;
}

static void handler()
{
    sigset_t const signal_set { set_sigset({SIGQUIT,SIGINT}) };
    int sig;

    while (true)
    {
        /* wait for any and all signals */
        sigwait( &signal_set, &sig );

        /* when we get this far, we've
        * caught a signal */

        switch (sig)
        {
        /* whatever you need to do on
        * SIGQUIT */
        case SIGQUIT:
            break;

        /* whatever you need to do on
        * SIGINT */
        case SIGINT:
            break;

        /* whatever you need to do for
        * other signals */
        default:
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Signal_Handler

std::thread Signal_Handler::create()
{
    std::thread thread { handler };

    return thread;
}

