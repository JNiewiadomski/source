class Signals
{
public:
    /**
     * @throws std::system_error on sigaction call failure
     */
    Signals();

    /**
     * @return True if a keyboard interrupt (^C) was received.
     */
    bool received_interrupt_from_keyboard() const;

    /**
     * Returns true if the console window size (number of rows or columns) was changed since the
     * last time this method was called.
     *
     * @return True if size was changed since last time called.
     */
    bool window_size_was_changed() const;
};

