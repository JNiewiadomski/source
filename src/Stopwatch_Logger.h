#pragma once

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <chrono>
#include <iostream>
#include <iomanip>
#include <memory>

// Usage samples:
//
//  #include "/home/vagrant/unix/src/Stopwatch_Logger.h"
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
//      sql.lap("do_2");
//  }
//
//  // Conditionally use Stopwatch_Logger.
//  {
//      std::unique_ptr<Stopwatch_Logger> swl;
//
//      if (condition)
//      {
//          swl = std::make_unique<Stopwatch_Logger>("conditional");
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

class Stopwatch_Logger
{
private:
    typedef std::chrono::steady_clock::time_point Time_Point;

public:
    Stopwatch_Logger(std::string const title, int64_t const threshold = 0)
        :
        m_log_prefix(build_log_pefix(title)),
        m_threshold(threshold)
    {
    }

    ~Stopwatch_Logger()
    {
        Time_Point const stop_time { now() };

        if (0 < duration(m_start_time, m_lap_time))
        {
            lap("stop", stop_time);
        }

        if (!m_message.empty())
        {
            m_message.append(", ");
        }

        m_message
            .append("Total=")
            .append(duration_as_string(m_start_time, stop_time))
            .append(" ms")
            ;

        if (duration(m_start_time, stop_time) > m_threshold)
        {
            std::cout << m_log_prefix << m_message << std::endl;
        }
    }

    Stopwatch_Logger & lap(std::string const name)
    {
        lap(name, now());
        return *this;
    }

    Stopwatch_Logger & log(std::string const log_message)
    {
        std::cout << m_log_prefix << log_message << std::endl;
        return *this;
    }

private:
    std::string build_log_pefix(std::string const title) const
    {
        std::ostringstream prefix;

        prefix << title << "[" << syscall(SYS_gettid) << "]:";

        return prefix.str();
    }

    void lap(std::string const name, Time_Point const new_lap_time)
    {
        if (!m_message.empty())
        {
            m_message.append(", ");
        }

        m_message
            .append(name)
            .append("=")
            .append(duration_as_string(m_lap_time, new_lap_time))
            .append(" ms")
            ;

        m_lap_time = { new_lap_time };
    };

    Time_Point now() const { return std::chrono::steady_clock::now(); }

    std::string duration_as_string(Time_Point const & t1, Time_Point const & t2) const
    {
        double const ms { static_cast<double>(duration(t1, t2)) / 1'000.0 };

        std::ostringstream o;
        o << std::fixed << std::setprecision(3) << ms;

        return o.str().c_str();
    }

    int64_t duration(Time_Point const & t1, Time_Point const & t2) const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }

    std::string const m_log_prefix;
    Time_Point const m_start_time { now() };
    Time_Point m_lap_time { m_start_time };
    std::string m_message;
    int64_t const m_threshold;
};

