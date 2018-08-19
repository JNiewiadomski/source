class Signals
{
public:
    Signals();
    ~Signals();

    /**
     * Returns true if the console window size (number of rows or columns) was changed since the
     * last time this method was called.
     *
     * @return True if size was changed since last time called.
     */
    bool window_size_was_changed() const;
};

