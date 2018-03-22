#include <wspp/util/sqlite/transaction.hpp>

using namespace std ;

namespace wspp { namespace util { namespace sqlite {

Transaction::Transaction(Connection &con): con_(con) {

    char *err_msg ;

    if ( sqlite3_exec (con_.handle(), "BEGIN", NULL, NULL, &err_msg) != SQLITE_OK ) {
        throw Exception(err_msg);
        sqlite3_free (err_msg);
    }

}

void Transaction::commit()
{

    char *err_msg ;

    if ( sqlite3_exec (con_.handle(), "COMMIT", NULL, NULL, &err_msg) != SQLITE_OK ) {
        throw Exception(err_msg);
        sqlite3_free (err_msg);
    }

}

void Transaction::rollback()
{
    char *err_msg ;

    if ( sqlite3_exec (con_.handle(), "ROLLBACK", NULL, NULL, &err_msg) != SQLITE_OK ) {
        throw Exception(err_msg);
        sqlite3_free (err_msg);
    }
}


} // namespace sqlite
} // namespace util
} // namespace wspp
