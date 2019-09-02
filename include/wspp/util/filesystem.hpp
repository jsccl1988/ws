#ifndef __UTIL_FILESYSTEM_HPP__
#define __UTIL_FILESYSTEM_HPP__

#include <string>

namespace wspp {
namespace util {
// read whole file to memory
std::string readFileToString(const std::string &fileName);
}}
#endif
