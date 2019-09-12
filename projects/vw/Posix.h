#include <string>

namespace Posix
{
    /**
     * Returns a pointer to a string that describes the error code passed in the argument errnum.
     * This function is thread safe.
     *
     * @param[in] errnum Error number corresponding to the error description string to return.
     *
     * @return The appropriate error description string, or an "Unknown error nnn" message if the
     *          error number is unknown.
     */
    std::string strerror_r(int const errnum);
}

