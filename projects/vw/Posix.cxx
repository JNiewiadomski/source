#include <cstring>

#include "Posix.h"

namespace Posix
{
    std::string strerror_r(int const errnum)
    {
        char error_buffer[BUFSIZ] {};
        return ::strerror_r(errnum, error_buffer, sizeof(error_buffer));
    }
}

