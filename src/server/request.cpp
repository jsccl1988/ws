#include "request.hpp"

#include <boost/regex.hpp>

using namespace std ;

namespace http {

bool Request::matches(const std::string &method, const std::string &rx) const
{
    boost::smatch m ;
    return ( method_ == method && boost::regex_match(path_, m, boost::regex(rx)) ) ;

}

bool Request::matches(const std::string &method, const std::string &rx, string &cap1) const
{
    boost::smatch m ;
    if ( method_ != method || !boost::regex_match(path_, m, boost::regex(rx)) )  return false ;
    if ( m.size() == 2 ) cap1 = m.str(1) ;
    return true ;
}

bool Request::matches(const std::string &method, const std::string &rx, string &cap1, string &cap2) const
{
    boost::smatch m ;
    if ( method_ != method || !boost::regex_match(path_, m, boost::regex(rx)) )  return false ;
    if ( m.size() == 3 ) {
        cap1 = m.str(1) ;
        cap2 = m.str(2) ;
    }
    return true ;
}


}
