#include <signal.h>

#include <atomic>

#include "Signals.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Private

static sighandler_t s_previous_SIGWINCH_handler {};

static std::atomic<bool> s_window_size_was_changed { true };

static void handle_window_change(int const /*signal*/)
{
    s_window_size_was_changed = { true };
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Signals

Signals::Signals()
{
    s_previous_SIGWINCH_handler = signal(SIGWINCH, handle_window_change);
}

Signals::~Signals()
{
    signal(SIGWINCH, s_previous_SIGWINCH_handler);
}

bool Signals::window_size_was_changed() const
{
    bool const rv { s_window_size_was_changed };

    s_window_size_was_changed = { false };

    return rv;
}

