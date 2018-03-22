#ifndef __PGSQL_STRING_CONV_HPP__
#define __PGSQL_STRING_CONV_HPP__

#include <wspp/database/types.hpp>
#include <string>


namespace wspp { namespace db {

template<typename T>
inline std::string pq_to_string(T val) {
    return std::to_string(val) ;
}

template<> inline std::string pq_to_string(const char *val) { return val; }

template<> inline std::string pq_to_string(const std::string &val) { return val; }

template<> inline std::string pq_to_string(std::string val) { return val; }

template<> inline std::string pq_to_string(char *val) { return val; }

template<> inline std::string pq_to_string(bool val) { return (val) ? "true" : "false"; }

} // namespace db
} // namespace wspp
#endif
