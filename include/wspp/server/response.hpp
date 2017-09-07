#ifndef HTTP_SERVER_REPLY_HPP
#define HTTP_SERVER_REPLY_HPP

#include <string>
#include <vector>

#include <wspp/util/dictionary.hpp>
#include <wspp/util/variant.hpp>

namespace wspp { namespace server {

using util::Variant ;
using util::Dictionary ;

/// A reply to be sent to a client.
struct Response
{
    /// The status of the reply.
    enum status_type
    {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } status_ = ok;

    /// The headers to be included in the reply.
    Dictionary headers_;

    /// The content to be sent in the reply.
    std::string content_;

    /// Get a stock reply.
    void stock_reply(status_type status);

    // this will fill in the reply for sending over a file payload

    void encode_file_data(const std::string &bytes,
                     const std::string &encoding,
                     const std::string &mime,
                     const time_t mod_time) ;

    void encode_file(const std::string &path_name,
                     const std::string &encoding = std::string(),
                     const std::string &mime = std::string()) ;

    void writeJSON(const std::string &json) ;
    void writeJSONVariant(const Variant &json) ;

    void write(const std::string &content, const std::string &mime = "text/html") ;
    void append(const std::string &content) ;

    void setCookie(const std::string &name, const std::string &value,
                   time_t expires = 0,
                   const std::string &path = std::string(),
                   const std::string &domain = std::string(),
                   bool secure = false,
                   bool http_only = false
                   ) ;

    template <class T>
    void append(const T &t) {
        std::ostringstream strm ;
        strm << t ;
        append(strm.str()) ;
    }

    // set header for content-type
    void setContentType(const std::string &mime);
    // set header for content-length because on the current content length
    void setContentLength() ;

    void setStatus(status_type st) { status_ = st ; }
};


} // namespace server
} // namespace wspp

#endif // HTTP_SERVER2_REPLY_HPP
