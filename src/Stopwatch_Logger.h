#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

// The Stopwatch_Logger class provides a straightforward way to time the duration of blocks of
// code. The start time is automatically set when the object is first instantiated. Lap times can
// be logged as desired throughout the code block, as can text annotations. The total time is
// logged when the object is destroyed.
//
// By default all output is logged to standard output. This is convenient from the following
// standpoints:
//
//  -   If binary is running as a service the contents is captured in syslog.
//
//  -   If binary is running as an application the contents is written to the console and can be
//      redirected.
//
// Usage samples:
//
//  #include <Stopwatch_Logger.h>
//
//  // Directly instantiate and use Stopwatch_Logger.
//  {
//      Stopwatch_Logger swl("direct");
//
//      do_1();
//      swl.lap("do_1");
//
//      swl.log("Log a message right now.");
//
//      do_2();
//      swl.lap("do_2");
//  }
//
//  // Conditionally use Stopwatch_Logger.
//  {
//      std::unique_ptr<Stopwatch_Logger> swl;
//
//      if (condition)
//      {
//          swl = { std::make_unique<Stopwatch_Logger>("conditional") };
//      }
//
//      do_1();
//      if (swl) swl->lap("do_1");
//
//      if (swl) swl->log("Log a message right now.");
//
//      do_2();
//      if (swl) swl->lap("do_2");
//  }
//
// Sample output:
//
//  direct[140597474780992]: Log a message right now.
//  direct[140597474780992]: do_1=0.001 ms, do_2=1462.334 ms, stop=3157.466 ms, Total=4619.801 ms
//
//  conditional[140597474780992]: Log a message right now.
//  conditional[140597474780992]: do_1=0.002 ms, do_2=3157.280 ms, stop=0.030 ms, Total=3157.313 ms

class Stopwatch_Logger
{
private:
    typedef std::chrono::steady_clock::time_point Time_Point;

    static std::string build_log_pefix(std::string const title)
    {
        std::ostringstream prefix;

        prefix << title << "[" << std::this_thread::get_id() << "]: ";

        return prefix.str();
    }

    static Time_Point now() { return std::chrono::steady_clock::now(); }

    static std::string duration_us_as_string(Time_Point const & t1, Time_Point const & t2)
    {
        double const ms { static_cast<double>(duration_us(t1, t2)) / 1'000.0 };

        std::ostringstream o;
        o << std::fixed << std::setprecision(3) << ms;

        return o.str().c_str();
    }

    static int64_t duration_us(Time_Point const & t1, Time_Point const & t2)
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }

public:
    /**
     * Constructor
     *
     * @param title The title will be included in the prefix of each logged line.
     * @param threshold_us Only log if total time lasted at least the specified microseconds.
     * @param out Stream where output will be logged.
     */
    Stopwatch_Logger(
        std::string const title,
        int64_t const threshold_us = 0,
        std::ostream & out = std::cout
        )
        :
        m_log_prefix { build_log_pefix(title) },
        m_threshold_us { threshold_us },
        m_out { out }
    {
    }

    /**
     * Destructor
     *
     * Content will be logged if the total time lasted at least the specified microseconds.
     */
    ~Stopwatch_Logger()
    {
        Time_Point const stop_time { now() };

        if (m_threshold_us <= duration_us(m_start_time, stop_time))
        {
            if (0 < duration_us(m_start_time, m_lap_time))
            {
                lap("stop", stop_time);
            }

            if (!m_message.empty())
            {
                m_message.append(", ");
            }

            m_message
                .append("Total=")
                .append(duration_us_as_string(m_start_time, stop_time))
                .append(" ms")
                ;

            m_out << m_log_prefix << m_message << std::endl;
        }
    }

    /**
     * Capture the current lap time. Nothing is actually logged until the destructor executes.
     *
     * @param name A name associated with the captured lap time.
     */
    Stopwatch_Logger & lap(std::string const name)
    {
        lap(name, now());
        return *this;
    }

    /**
     * Immediately log the message to the output stream.
     *
     * @param log_message Message, plus prefix, to log immediately to the output stream.
     */
    Stopwatch_Logger & log(std::string const log_message)
    {
        m_out << m_log_prefix << log_message << std::endl;
        return *this;
    }

private:
    void lap(std::string const name, Time_Point const new_lap_time)
    {
        if (!m_message.empty())
        {
            m_message.append(", ");
        }

        m_message
            .append(name)
            .append("=")
            .append(duration_us_as_string(m_lap_time, new_lap_time))
            .append(" ms")
            ;

        m_lap_time = { new_lap_time };
    };

    std::string const m_log_prefix;
    Time_Point const m_start_time { now() };
    Time_Point m_lap_time { m_start_time };
    std::string m_message;
    int64_t const m_threshold_us;
    std::ostream & m_out;
};

