#include <wspp/util/sqlite/stmt.hpp>
#include <wspp/util/sqlite/exception.hpp>

#include <boost/format.hpp>

#include <cassert>
#include <cstring>

using namespace std ;

namespace wspp { namespace util { namespace sqlite {


Stmt::Stmt(sqlite3 *con, const string &sql): last_arg_idx_(0)
{
    assert(con) ;

    const char * tail = 0;

    if ( sqlite3_prepare_v2(con, sql.c_str(), -1, &handle_ ,&tail) != SQLITE_OK )
        throw Exception(con) ;


    int num_fields = sqlite3_column_count(handle_);

    for( int index = 0; index < num_fields; index++ ) {
        const char* field_name = sqlite3_column_name(handle_, index);
        field_map_[field_name] = index ;
    }
}

int Stmt::columnIdx(const string &name) const {
    auto it = field_map_.find(name) ;
    if ( it != field_map_.end() ) return (*it).second ;
    else return -1 ;
}

Stmt::~Stmt() {
    sqlite3_finalize(handle_) ;
}

void Stmt::clear()
{
    check();
    if ( sqlite3_reset(handle_) != SQLITE_OK ) throwStmtException();
    //    if ( sqlite3_clear_bindings(handle_) != SQLITE_OK ) throwStmtException() ;
    last_arg_idx_ = 0 ;
}

void Stmt::finalize() {
    check();
    if ( sqlite3_finalize(handle_) != SQLITE_OK ) throwStmtException();
    handle_ = 0;
}

void Stmt::throwStmtException()
{
    throw Exception(sqlite3_db_handle(handle_)) ;
}

void Stmt::check() const {
    if( !handle_ ) throw Exception("Stmt has not been compiled.");
}

/* returns true if the command returned data */

bool Stmt::step() {
    check() ;

    switch( sqlite3_step(handle_) ) {
    case SQLITE_ROW:
        return true;
    case SQLITE_DONE:
        return false;
    default:
        throwStmtException();
    }
    return false;
}

Stmt &Stmt::bind(int idx, const NullType &) {
    check();
    if ( sqlite3_bind_null(handle_, idx) != SQLITE_OK ) throwStmtException();
    return *this ;
}


Stmt &Stmt::bind(int idx, unsigned char v) {
    check();
    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, char v) {
    check();
    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, int v) {
    check();
    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, unsigned int v) {
    check();
    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, unsigned short int v) {
    check();
    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, short int v) {
    check();
    if ( sqlite3_bind_int(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, long int v) {
    check();
    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, unsigned long int v) {
    check();
    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, long long int v){
    check();
    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, unsigned long long int v){
    check();
    if ( sqlite3_bind_int64(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, double v){
    check() ;
    if ( sqlite3_bind_double(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, float v){
    check() ;
    if ( sqlite3_bind_double(handle_, idx, v) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, const string &v){
    check() ;
    if ( sqlite3_bind_text(handle_, idx, v.c_str(), int(v.size()), SQLITE_TRANSIENT ) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, const Blob &blob){
    check() ;
    if ( sqlite3_bind_blob(handle_, idx, blob.data(), blob.size(), SQLITE_TRANSIENT ) != SQLITE_OK ) throwStmtException();
    return *this ;
}

Stmt &Stmt::bind(int idx, const char *v){
    check() ;
    if ( sqlite3_bind_text(handle_, idx, v, strlen(v), SQLITE_TRANSIENT ) != SQLITE_OK ) throwStmtException();
    return *this ;
}


int Stmt::columns() const {
    check() ;
    return ( sqlite3_data_count(handle_) ) ;
}

const char *Stmt::columnName(int idx) const {
   check() ;
   const char *name = sqlite3_column_name(handle_, idx)  ;
   if ( name == nullptr ) throw Exception(str(boost::format("There is no column with index %d") % idx)) ;
   else return name ;
}

int Stmt::columnType(int idx) const {
    check() ;
    return sqlite3_column_type(handle_, idx);
}

int Stmt::columnBytes(int idx) const {
    check() ;
    return sqlite3_column_bytes(handle_, idx);
}


template<>
int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int(handle_, idx);
}

template<>
unsigned int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int(handle_, idx);
}

template<>
short int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int(handle_, idx);
}

template<>
unsigned short int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int(handle_, idx);
}

template<>
long int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int64(handle_, idx);
}

template<>
unsigned long int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int64(handle_, idx);
}

template<>
bool Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int(handle_, idx);
}

template<>
double Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_double(handle_, idx);
}

template<>
float Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_double(handle_, idx);
}

template<>
long long int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int64(handle_, idx);
}

template<>
unsigned long long int Stmt::get(int idx) const
{
    check() ;
    return sqlite3_column_int64(handle_, idx);
}

template<>
char const* Stmt::get(int idx) const
{
    return reinterpret_cast<char const*>(sqlite3_column_text(handle_, idx));
}

template<>
std::string Stmt::get(int idx) const
{
    const char *res = reinterpret_cast<char const*>(sqlite3_column_text(handle_, idx));
    if ( res == nullptr ) return string() ;
    else return res ;
}

template<>
Blob Stmt::get(int idx) const
{
    check() ;
    const void *data = sqlite3_column_blob(handle_, idx);
    int bytes = sqlite3_column_bytes(handle_, idx) ;
    return Blob((const char *)data, bytes) ;
}

template<>
void Stmt::read(int idx, int &val) const
{
    check() ;
    val = sqlite3_column_int(handle_, idx);
}

template<>
void Stmt::read(int idx, unsigned int &val) const
{
    check() ;
    val = sqlite3_column_int(handle_, idx);
}

template<>
void Stmt::read(int idx, short int &val) const
{
    check() ;
    val = sqlite3_column_int(handle_, idx);
}

template<>
void Stmt::read(int idx, unsigned short &val) const
{
    check() ;
    val = sqlite3_column_int(handle_, idx);
}

template<>
void Stmt::read(int idx, long int &val) const
{
    check() ;
    val = sqlite3_column_int64(handle_, idx);
}

template<>
void Stmt::read(int idx, unsigned long &val) const
{
    check() ;
    val = sqlite3_column_int64(handle_, idx);
}

template<>
void Stmt::read(int idx, bool &v) const
{
    check() ;
    v = sqlite3_column_int(handle_, idx);
}

template<>
void Stmt::read(int idx, double &val) const
{
    check() ;
    val = sqlite3_column_double(handle_, idx);
}

template<>
void Stmt::read(int idx, float &val) const
{
    check() ;
    val = sqlite3_column_double(handle_, idx);
}

template<>
void Stmt::read(int idx, long long int &val) const
{
    check() ;
    val = sqlite3_column_int64(handle_, idx);
}

template<>
void Stmt::read(int idx, unsigned long long &val) const
{
    check() ;
    val = sqlite3_column_int64(handle_, idx);
}


template<>
void Stmt::read(int idx, std::string &val) const
{
    const char *res = reinterpret_cast<char const*>(sqlite3_column_text(handle_, idx));
    if ( res == nullptr ) return  ;
    val.assign(res) ;
}

template<>
void Stmt::read(int idx, Blob &blob) const
{
    check() ;
    const void *data = sqlite3_column_blob(handle_, idx);
    int bytes = sqlite3_column_bytes(handle_, idx) ;
    blob = Blob((const char *)data, bytes) ;
}

} // namespace sqlite
} // namespace util
} // namespace wspp
