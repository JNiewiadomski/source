#include <signal.h>

#include <atomic>
#include <thread>
#include <vector>

class Signal_Handler
{
public:
    /**
     * Constructor.
     *
     * @param[in] signal_numbers Signals that will be handled by Signal_Handler.
     */
    Signal_Handler(std::vector<int> const & signal_numbers);

    /**
     * Destructor
     */
    virtual ~Signal_Handler();

    /**
     * Callback invoked each time a handled signal is received.
     *
     * @param[in] signal_number Number of signal received.
     */
    virtual void on_signal(int const signal_number) = 0;

private:
    void handler();

    std::thread m_thread {};
    sigset_t m_original_sigset {};
    std::atomic<bool> m_stop_hander { false };

    sigset_t const m_sigset;
};

