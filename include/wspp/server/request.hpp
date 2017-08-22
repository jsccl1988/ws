//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_REQUEST_HPP
#define HTTP_SERVER_REQUEST_HPP

#include <string>
#include <vector>

#include <wspp/util/dictionary.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

namespace wspp {

class UriPatternMatcher ;

/// A request received from a client.
class Request
{
public:

    // matches the request method with one of the specified methods (list of methods delimeted by | character)
    // matches the request uri to the pattern
    // pattern is the uri pattern in the form /<pat1>/<pat2>/<pat3> ... /<patn>/
    // where each sub-pattern has the format  <prefix>{<param>[:<verifier>]}[?]<suffix>
    // e.g. /user/{id:n}/{action:show|hide}/
    // If the match is succesfull the method returns true and recovers the named parameters values (e.g. id, action).
    // The verifier can be one of 'w' (word), 'a' (alphanumeric), 'n' (numeric), '*' (any except /) and '**' (any) or otherwise it is assumed to be
    // a verbatim regular expression (e.g. 'show|hide')

    bool matches(const std::string &method, const std::string &pattern, Dictionary &attributes) const ;
    bool matches(const std::string &method, const std::string &pattern) const;

public:
    Dictionary SERVER_ ; // Server variables
    Dictionary GET_ ;	 // Query variables for GET requests
    Dictionary POST_ ;   // Post variables for POST requests
    Dictionary COOKIE_ ; // Cookies

    struct UploadedFile {
        std::string orig_name_ ;	// The original filename
        std::string server_path_ ; // The path of a local temporary copy of the uploaded file
        std::string mime_ ;		// MIME information of the uploaded file
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


} // namespace http

#endif // HTTP_SERVER2_REQUEST_HPP
