#include <wspp/server/fs_session_handler.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <chrono>
#include <random>

using namespace std ;
using namespace wspp::util ;
namespace fs = boost::filesystem ;

namespace wspp { namespace server {

FileSystemSessionHandler::FileSystemSessionHandler(const std::string &db_file) {

    // open database

    if ( db_file.empty() ) {
        fs::path tmp = fs::temp_directory_path() / "wsx_session.sqlite" ;
        db_.open(tmp.native(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX ) ;
    }
    else {
        fs::path p(db_file) ;
        if ( !fs::exists(p) ) {
            boost::system::error_code ec ;
            fs::create_directories(p.parent_path(), ec) ;
            db_.open(p.native(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX ) ;
        }
    }

    db_.exec("PRAGMA auto_vacuum = 1;\
                PRAGMA journal_mode = WAL;\
                PRAGMA synchronous = NORMAL;\
                CREATE TABLE IF NOT EXISTS sessions ( sid TEXT PRIMARY KEY NOT NULL, data BLOB DEFAULT NULL, ts INTEGER NOT NULL );\
                CREATE UNIQUE INDEX IF NOT EXISTS sessions_index ON sessions (sid);") ;
}

bool FileSystemSessionHandler::open() {
    return true ;
}

bool FileSystemSessionHandler::close() {
    return true ;
}

string FileSystemSessionHandler::uniqueSID() {
    int max_tries = 4 ;
    while ( max_tries > 0 ) {
        string sid = generateSID() ;
        if ( !sid.empty() && !contains(sid) ) return sid ;
        else max_tries -- ;
    }
    return string() ;
}

enum
{
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul,
    O32_PDP_ENDIAN = 0x01000302ul
};


static bool platform_is_little_endian() {
    static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order =
    { { 0, 1, 2, 3 } };

    return ( o32_host_order.value == 0x03020100ul ) ;
}

static void byte_swap_32(uint32_t &data)
{
    union u {uint32_t v; uint8_t c[4];};
    u un, vn;
    un.v = data ;
    vn.c[0]=un.c[3];
    vn.c[1]=un.c[2];
    vn.c[2]=un.c[1];
    vn.c[3]=un.c[0];
    data = vn.v ;
}

static uint32_t read_uint32(istream &strm) {
    uint32_t i ;
    if ( platform_is_little_endian() )
        strm.read((char *)&i, 4) ;
    else {
        strm.read((char *)&i, 4) ;
        byte_swap_32(i) ;
    }
    return i ;
}

static void write_uint32(ostream &strm, uint32_t i) {
    if ( platform_is_little_endian() )
        strm.write((char *)&i, 4) ;
    else {
        byte_swap_32(i) ;
        strm.write((char *)&i, 4) ;
    }
}

static void write_string(ostream &strm, const string &str) {
    write_uint32(strm, str.length()) ;
    strm.write(str.data(), str.length()) ;
}

static string read_string(istream &strm) {
    uint32_t len = read_uint32(strm) ;
    string res ;
    res.resize(len) ;
    strm.read((char *)res.data(), len) ;
    return res ;
}

string FileSystemSessionHandler::serializeData(const Dictionary &data)
{
    ostringstream strm(ios::out | ios::binary) ;

    write_uint32(strm, data.size()) ;

    for( auto &p: data ) {
        write_string(strm, p.first) ;
        write_string(strm, p.second) ;
    }

    return strm.str() ;
}

void FileSystemSessionHandler::deserializeData(const string &data, Dictionary &dict)
{
    istringstream strm(data, ios::in | ios::binary) ;

    uint32_t len = read_uint32(strm) ;

    for( uint i=0 ; i<len ; i++ ) {
        string key = read_string(strm) ;
        string val = read_string(strm) ;
        dict[key] = val ;
    }
}

bool FileSystemSessionHandler::writeSessionData(const string &id, const string &data)
{
    try {

        sqlite::Transaction trans(db_) ;
        sqlite::Statement cmd(db_, "REPLACE INTO sessions (sid, data, ts) VALUES (?, ?, ?)",
                          id,
                          sqlite::Blob(data.data(), data.size()),
                          (uint64_t)std::chrono::system_clock::now().time_since_epoch().count()) ;
        cmd.exec() ;
        trans.commit() ;

        return true ;
    }
    catch ( sqlite::Exception & ) {
       return false ;
    }

}

bool FileSystemSessionHandler::readSessionData(const string &id, string &data)
{
    try {
        sqlite::Query q(db_, "SELECT data FROM sessions WHERE sid = ? LIMIT 1", id) ;
        sqlite::QueryResult res = q.exec() ;
        sqlite::Blob bdata = res.get<sqlite::Blob>(0) ;
        data.assign(bdata.data(), bdata.size()) ;
        return true ;
    }
    catch ( sqlite::Exception &e ) {
        cerr << e.what() << endl ;
       return false ;
    }
}

bool FileSystemSessionHandler::contains(const string &id) {
    sqlite::Query q(db_, "SELECT sid FROM sessions WHERE sid = ? LIMIT 1", id) ;
    sqlite::QueryResult res = q.exec() ;
    return res ;
}

bool FileSystemSessionHandler::write(const Session &session) {
    string id = session.id() ;
    string data = serializeData(session.data()) ;
    return writeSessionData(id, data) ;
}


bool FileSystemSessionHandler::read(Session &session) {
    string id = session.id(), data ;
    if ( !readSessionData(id, data) ) return false ;
    deserializeData(data, session.data()) ;
    gc() ;
}

// php like session garbage collection

void FileSystemSessionHandler::gc() {
    static const auto session_entry_max_lifetime = std::chrono::minutes(60) ;
    static const uint64_t session_gc_probability = 1 ;
    static const uint64_t session_gc_divisor = 1000 ;
    static std::random_device session_gc_rnd_device ;
    static std::default_random_engine session_gc_rnd_engine(session_gc_rnd_device());

    std::uniform_int_distribution<uint64_t> uniform_dist(1, session_gc_divisor);
    uint64_t p =  uniform_dist(session_gc_rnd_engine) ;

    if ( p  <= session_gc_probability ) {

        // delete old records

        auto t = std::chrono::system_clock::now() - session_entry_max_lifetime ;

        sqlite::Statement cmd(db_, "DELETE FROM sessions WHERE ts < ?",
                      (uint64_t)t.time_since_epoch().count()) ;
        cmd.exec() ;
    }

}


} // namespace server
} // namespace wspp
