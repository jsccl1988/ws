#ifndef __WSPP_UTIL_RANDOM_HPP__
#define __WSPP_UTIL_RANDOM_HPP__

#include <string>

namespace wspp { namespace util {

// generate random bytes string of specified length
std::string randomBytes(size_t len = 32) ;

// convert binary string to hex
std::string binToHex(const std::string &src) ;

// to base64 string from binary
std::string encodeBase64(const std::string &src) ;

// from base64 string to binary
std::string decodeBase64(const std::string &src) ;

// create a hash (combined key and salt) from a password. result string is binary and thus should be appropriately encoded before storing to database
std::string passwordHash(const std::string &password, size_t iterations = 1000) ;

// validate hash and password combination
bool passwordVerify(const std::string &password, const std::string &hash) ;

// hashes input string using SHA256 alogorithm, returning binary string
std::string hashSHA256(const std::string &src) ;

bool hashCompare(const std::string &s1, const std::string &s2) ;

} }


#endif
