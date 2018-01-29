#ifndef __SERVER_REQUEST_HPP__
#define __SERVER_REQUEST_HPP__

#include <string>
#include <vector>

#include <wspp/util/dictionary.hpp>
#include <wspp/server/route.hpp>

namespace wspp { namespace server {

using util::Dictionary ;

/// A request received from a client.
class Request
{
public:
    // helper function to match a request with a Route pattern (see Route)
    bool matches(const std::string &method, const Route &pattern, Dictionary &attributes) const ;
    bool matches(const std::string &method, const Route &pattern) const;
    bool matches(const std::string &method, const std::string &pattern, Dictionary &attributes) const ;
    bool matches(const std::string &method, const std::string &pattern) const;

    bool supportsGzip() ;

public:
    Dictionary SERVER_ ; // Server variables
    Dictionary GET_ ;	 // Query variables for GET requests
    Dictionary POST_ ;   // Post variables for POST requests
    Dictionary COOKIE_ ; // Cookies

    struct UploadedFile {
        std::string name_ ;	// The original filename
        std::string path_ ; // The path of a local temporary copy of the uploaded file
        std::string mime_ ;	// MIME information of the uploaded file
        size_t size_ ;
        std::string data_ ; // This member variable contains the file contents for small files
    } ;

    std::map<std::string, UploadedFile> FILE_ ;	// Uploaded files

    // This is content sent using POST with Content-Type other than
    // x-www-form-urlencoded or multipart-form-data e.g. text/xml

    std::string content_ ;
    std::string content_type_ ;

    std::string method_;
    std::string path_;
    std::string query_ ;
    std::string protocol_ ;


private:

    bool matchesMethod(const std::string &method) const ;

    std::string getCleanPath(const std::string &path) const ;
};


} // namespace server
} // namespave wspp

#endif
