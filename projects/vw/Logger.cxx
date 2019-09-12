#include "Logger.h"
#include "Posix.h"

namespace Logger
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Private

    static std::ostream * s_out { &std::cout };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Public

    //---------------------------------------------------------------------------------------------
    void log(
        std::string const & prefix,
        std::string const & message,
        std::string const & file,
        int const line
        )
    {
        if (!prefix.empty())
        {
            *s_out << prefix << ": ";
        }

        if (!message.empty())
        {
            *s_out << message << ": ";
        }

        if (!file.empty())
        {
            *s_out << "Logged at " << file << ":" << line;
        }

        *s_out << std::endl;
    }

    //---------------------------------------------------------------------------------------------
    void log_and_abort(
        std::string const & prefix,
        std::string const & message,
        std::string const & file,
        int const line
        )
    {
        log(prefix, message, file, line);
        abort();
    }

    //---------------------------------------------------------------------------------------------
    void log_and_abort(
        std::string const & prefix,
        std::string const & name,
        int const status,
        std::string const & file,
        int const line
        )
    {
        std::string message { "System call " };

        if (!name.empty())
        {
            message.append("to ").append(name);
        }

        message
            .append("failed: \"")
            .append(Posix::strerror_r(status))
            .append("\" (errno=")
            .append(std::to_string(status))
            .append(")")
            ;

        log_and_abort(prefix, message, file, line);
    }

    //---------------------------------------------------------------------------------------------
    void set_stream(std::ostream & out)
    {
        s_out = { &out };
    }
}

