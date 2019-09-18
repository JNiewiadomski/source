#include "Logger.h"
#include "Signal_Handler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Private

//-------------------------------------------------------------------------------------------------
static sigset_t set_sigset(std::vector<int> const & signal_numbers)
{
    sigset_t signal_set {};

    if (sigemptyset(&signal_set) != 0)
    {
        Logger::log_and_abort("Signal_Handler", "sigemptyset", errno, __FILE__, __LINE__);
    }

    for (int const & signal_number : signal_numbers)
    {
        if (sigaddset(&signal_set, signal_number) != 0)
        {
            Logger::log_and_abort("Signal_Handler", "sigaddset", errno, __FILE__, __LINE__);
        }
    }

    return signal_set;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Signal_Handler

//-------------------------------------------------------------------------------------------------
Signal_Handler::Signal_Handler(std::vector<int> const & signal_numbers)
    :
    m_sigset { set_sigset(signal_numbers) }
{
    // Set the signal mask for the current thread. New threads inherit the signal mask of the
    // thread that created it. A signal mask can effectively be set for an entire process by
    // calling pthread_sigmask() before any threads are created.
    {
        int const status { pthread_sigmask(SIG_BLOCK, &m_sigset, &m_original_sigset) };

        if (status != 0)
        {
            Logger::log_and_abort("Signal_Handler", "pthread_sigmask", status, __FILE__, __LINE__);
        }
    }

    // Everything is set up. Start a new thread that will invoke handler().
    m_thread =
    {
        std::thread { &Signal_Handler::handler, this }
    };
}

//-------------------------------------------------------------------------------------------------
Signal_Handler::~Signal_Handler()
{
    m_stop_hander = { true };
    m_thread.join();

    // Restore the original sigset.
    {
        int const status { pthread_sigmask(SIG_BLOCK, &m_original_sigset, nullptr) };

        if (status != 0)
        {
            Logger::log_and_abort("Signal_Handler", "pthread_sigmask", status, __FILE__, __LINE__);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void Signal_Handler::handler()
{
    timespec const timeout { 1 /*seconds*/, 0 /*nanoseconds*/ };

    while (!m_stop_hander)
    {
        int const signal_number { sigtimedwait(&m_sigset, nullptr, &timeout) };

        if (signal_number == -1)
        {
            if (errno == EINVAL)
            {
                Logger::log_and_abort("Signal_Handler", "sigtimedwait", errno, __FILE__, __LINE__);
            }
        }
        else
        {
            on_signal(signal_number);
        }
    }
}

