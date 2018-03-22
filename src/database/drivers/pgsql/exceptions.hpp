#ifndef __PQ_EXCEPTIONS_HPP__
#define __PQ_EXCEPTIONS_HPP__

#include <wspp/database/exception.hpp>

#include <libpq-fe.h>

namespace wspp { namespace db {

class PQException: public Exception {
public:
    PQException(PGconn *handle): Exception(PQerrorMessage(handle)) {}
};

} // namespace db
} // namespace wspp

#endif
