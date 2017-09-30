#include <wspp/util/sqlite/query_result.hpp>

using namespace std ;

namespace wspp { namespace util { namespace sqlite {

QueryResult::QueryResult(std::shared_ptr<Stmt> cmd): cmd_(cmd) {
    next() ;
}

int QueryResult::columns() const {
    return cmd_->columns() ;
}

string QueryResult::columnName(int idx) const {
    return cmd_->columnName(idx) ;
}

int QueryResult::columnIdx(const string &name) const {
    cmd_->check() ;
    return cmd_->columnIdx(name) ;
}

void QueryResult::next() {
    empty_ = !cmd_->step() ;
}

int QueryResult::columnType(int idx) const {
    return cmd_->columnType(idx) ;
}

int QueryResult::columnBytes(int idx) const {
    return cmd_->columnBytes(idx) ;
}

bool QueryResult::hasColumn(const string &name) const {
    return columnIdx(name) >= 0 ;
}

QueryResult::iterator::iterator(QueryResult &res, bool at_end): qres_(res), at_end_(at_end), current_(new Row(qres_)) {}

Dictionary QueryResult::getAll() const
{
    Dictionary res ;
    for( int i=0 ; i<columns() ; i++ ) {
        res.add(columnName(i), get<string>(i) ) ;
    }
    return res ;
}

} // namespace sqlite
} // namespace util
} // namespace wspp
