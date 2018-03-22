#ifndef __PQ_QUERY_RESULT_HANDLE_HPP__
#define __PQ_QUERY_RESULT_HANDLE_HPP__

#include <wspp/database/query_result_handle.hpp>
#include <wspp/database/types.hpp>

#include "statement.hpp"

namespace wspp {
namespace db {

class PQQueryResultHandle: public QueryResultHandle {
public:
    PQQueryResultHandle(const std::shared_ptr<PQStatementHandle> &stmt);

    ~PQQueryResultHandle() {}

    int at() const override {
        return pos_ ;
    }

    bool next() override ;

    int columns() const override ;

    int columnType(int idx) const override ;

    std::string columnName(int idx) const override ;

    int columnIndex(const std::string &name) const override ;

    bool columnIsNull(int idx) const override ;

    void read(int idx, int &val) const override ;
    void read(int idx, unsigned int &val) const override ;
    void read(int idx, short int &val) const override ;
    void read(int idx, unsigned short int &val) const override ;
    void read(int idx, long int &val) const override ;
    void read(int idx, unsigned long int &val) const override ;
    void read(int idx, bool &val) const override ;
    void read(int idx, double &val) const override ;
    void read(int idx, float &val) const override ;
    void read(int idx, long long int &val) const override ;
    void read(int idx, unsigned long long int &val) const override ;
    void read(int idx, std::string &val) const override ;
    void read(int idx, Blob &val) const override ;

    void reset() override;
private:

    void check_has_row() const ;

    std::shared_ptr<PQStatementHandle> stmt_ ;
    int pos_ = -1, num_rows ;
} ;

}
}

#endif