#include <thread>

namespace Signal_Handler
{
    /**
     * Create and start the Signal_Handler thread.
     *
     * @return Thread that is handling the signal processing.
     */
    std::thread create();

    /**
     * @return True if a keyboard interrupt (^C) was received.
     */
    bool received_interrupt_from_keyboard();

    /**
     * Returns true if the console window size (number of rows or columns) was changed since the
     * last time this method was called.
     *
     * @return True if size was changed since last time called.
     */
    bool window_size_was_changed();
}

