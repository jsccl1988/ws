#include <wspp/util/filesystem.hpp>

#include <fstream>

using namespace std ;

namespace wspp { namespace util {

string readFileToString(const string &fileName)
{
    ifstream strm(fileName) ;

    strm.seekg(0, ios::end);
    size_t length = strm.tellg();
    strm.seekg(0,std::ios::beg);

    string res ;
    res.resize(length) ;
    strm.read(&res[0], length) ;
    return res ;
}





}
}
