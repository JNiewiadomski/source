#include <stdexcept> // std::runtime_error

// Documentation
// https://www.sqlite.org/docs.html
//
#include "sqlite3.h"

class SQLite_Exception : public std::runtime_error
{
public:
    SQLite_Exception(std::string const & what, int const rc)
        :
        std::runtime_error { what },
        m_return_code { rc }
    {
    }

    int get_return_code() const { return m_return_code; }

private:
    int const m_return_code;
};

class SQLite
{
public:
    SQLite()
    {
        initialize();
    }

    ~SQLite()
    {
        try
        {
            shutdown();
        }
        catch (...)
        {
            // Don't allow exceptions to escape.
        }
    }

private:
    void initialize()
    {
        // The sqlite3_initialize() routine returns SQLITE_OK on success. If for some reason,
        // sqlite3_initialize() is unable to initialize the library (perhaps it is unable to
        // allocate a needed resource such as a mutex) it returns an error code other than
        // SQLITE_OK.
        //
        // https://www.sqlite.org/c3ref/initialize.html
        //
        int const rc { sqlite3_initialize() };

        if (rc != SQLITE_OK)
        {
            throw SQLite_Exception("sqlite3_initialize", rc);
        }
    }

    void shutdown()
    {
        // The sqlite3_initialize() interface is threadsafe, but sqlite3_shutdown() is not. The
        // sqlite3_shutdown() interface must only be called from a single thread. All open database
        // connections must be closed and all other SQLite resources must be deallocated prior to
        // invoking sqlite3_shutdown().
        //
        // https://www.sqlite.org/c3ref/initialize.html
        //
        int const rc { sqlite3_shutdown() };

        if (rc != SQLITE_OK)
        {
            throw SQLite_Exception("sqlite3_shutdown", rc);
        }
    }
};

int main()
{
    SQLite sqlite;

    return 0;
}

