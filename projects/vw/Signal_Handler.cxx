#include <signal.h>

#include <atomic>
#include <vector>

#include "Logger.h"
#include "Signal_Handler.h"

namespace Signal_Handler
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Private

    static std::atomic<bool> volatile s_received_interrupt_from_keyboard { false };
    static std::atomic<bool> volatile s_window_size_was_changed { true };

    //---------------------------------------------------------------------------------------------
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

    //---------------------------------------------------------------------------------------------
    static void handler()
    {
        // Set of signals that will be handled.
        sigset_t const signal_set { set_sigset({SIGINT,SIGWINCH}) };

        while (true)
        {
            int sig {};

            if (sigwait(&signal_set, &sig) != 0)
            {
                Logger::log_and_abort("Signal_Handler", "sigwait", errno, __FILE__, __LINE__);
            }

            switch (sig)
            {
            case SIGINT:
                s_received_interrupt_from_keyboard = { true };
                break;

            case SIGWINCH:
                s_window_size_was_changed = { true };
                break;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Signal_Handler

    //---------------------------------------------------------------------------------------------
    std::thread create()
    {
        return std::thread { handler };
    }

    bool received_interrupt_from_keyboard()
    {
        return s_received_interrupt_from_keyboard;
    }

    bool window_size_was_changed()
    {
        bool const rv { s_window_size_was_changed };

        s_window_size_was_changed = { false };

        return rv;
    }
}

