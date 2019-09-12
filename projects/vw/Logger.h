#include <iostream>
#include <string>

namespace Logger
{
    void log(
        std::string const & prefix,
        std::string const & message,
        std::string const & file,
        int const line
        );

    void log_and_abort(
        std::string const & prefix,
        std::string const & message,
        std::string const & file,
        int const line
        );

    void log_and_abort(
        std::string const & prefix,
        std::string const & name,
        int const status,
        std::string const & file,
        int const line
        );

    void set_stream(std::ostream & out);
}

