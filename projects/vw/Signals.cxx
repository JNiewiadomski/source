#include <signal.h>

#include <atomic>
#include <string>
#include <system_error>

#include "Signals.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Private

typedef void (*Signal_Handler_Function)(int const signum);

static void handle_signal(int const signum, Signal_Handler_Function const handler)
{
    // sigaction
    // http://man7.org/linux/man-pages/man2/sigaction.2.html

    struct sigaction sa {};

    sa.sa_handler = { handler };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = { 0 };

    if (sigaction(signum, &sa, nullptr) != 0)
    {
        std::string message;

        message
            .append("Call to sigaction failed (signum=")
            .append(std::to_string(signum))
            .append(")")
            ;

        throw std::system_error(errno, std::generic_category(), message);
    }
}

static std::atomic<bool> volatile s_received_interrupt_from_keyboard { false };
static std::atomic<bool> volatile s_window_size_was_changed { true };

static void handle_keyboard_interrupt(int const signum)
{
    if (signum == SIGINT)
    {
        s_received_interrupt_from_keyboard = { true };
    }
}

static void handle_window_change(int const signum)
{
    if (signum == SIGWINCH)
    {
        s_window_size_was_changed = { true };
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Signals

Signals::Signals()
{
    // Interrupt from keyboard (^C)
    handle_signal(SIGINT, handle_keyboard_interrupt);

    // Window resize signal (4.3BSD, Sun)
    handle_signal(SIGWINCH, handle_window_change);
}

bool Signals::received_interrupt_from_keyboard() const
{
    return s_received_interrupt_from_keyboard;
}

bool Signals::window_size_was_changed() const
{
    bool const rv { s_window_size_was_changed };

    s_window_size_was_changed = { false };

    return rv;
}

