#include <sys/ioctl.h>

#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "Posix.h"
#include "Signal_Handler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// My_Signal_Handler

class My_Signal_Handler : public Signal_Handler
{
public:
    My_Signal_Handler()
        :
        Signal_Handler({SIGINT,SIGWINCH})
    {
    }

    ~My_Signal_Handler() override
    {
    }

    void on_signal(int const signal_number) override
    {
        switch (signal_number)
        {
        case SIGINT:
            m_received_interrupt_from_keyboard = { true };
            break;

        case SIGWINCH:
            m_window_size_was_changed = { true };
            break;
        }
    }

    /**
     * @return True if a keyboard interrupt (^C) was received.
     */
    bool received_interrupt_from_keyboard()
    {
        return m_received_interrupt_from_keyboard;
    }

    /**
     * Returns true if the console window size (number of rows or columns) was changed since the
     * last time this method was called.
     *
     * @return True if size was changed since last time called.
     */
    bool window_size_was_changed()
    {
        bool const rv { m_window_size_was_changed };

        m_window_size_was_changed = { false };

        return rv;
    }

private:
    bool m_received_interrupt_from_keyboard { false };
    bool m_window_size_was_changed { true };
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Private

static My_Signal_Handler g_signal_handler;

static std::string box(size_t const rows, size_t const columns)
{
    std::string line;
    if (2 < columns)
    {
        for (size_t i=0; i<columns-2; ++i)
        {
            line.append(u8"\u2550");
        }
    }

    std::string box;

    box.append(u8"\u2554").append(line).append(u8"\u2557");
    {
        std::string middle;
        middle.append(u8"\u2551").append(std::string(columns-2, ' ')).append(u8"\u2551");
        if (2 < rows)
        {
            for (size_t i=0; i<rows-2; ++i)
            {
                box.append(middle);
            }
        }
    }
    box.append(u8"\u255A").append(line); //.append(u8"\u255D");

    return box;
}

static void print_window_size()
{
    if (g_signal_handler.window_size_was_changed())
    {
        winsize w {};

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        {
            std::cout << "Call to ioctl failed: " << Posix::strerror_r(errno) << std::endl;
        }

        //std::cout << "\x1B[2K\x1B[0G";
        //std::cout << "Lines=" << w.ws_row << ", Columns=" << w.ws_col << std::flush;

        //std::string const s { u8"\u2560" };
        //std::cout << " " << s << std::flush;

        std::cout << "\x1B[36;40m" << "\x1B[2J\x1B[0;0H" << box(w.ws_row, w.ws_col) << "\x1B[97;40m" << std::flush;
        //std::cout << "Lines=" << w.ws_row << ", Columns=" << w.ws_col << std::flush;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// main

int main()
{
    while (!g_signal_handler.received_interrupt_from_keyboard())
    {
        print_window_size();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\n" << "Done." << std::endl;

    return 0;
}

