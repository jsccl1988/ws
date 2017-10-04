//
// RequestParser.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// adopted from C++ wrapper of nodejs parser
// https://github.com/AndreLouisCaron/httpxx

#include <wspp/server/detail/request_parser.hpp>
#include <wspp/server/request.hpp>

#include <algorithm>
#include <cstring>
#include <utility>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/iostreams/stream.hpp>


using namespace std ;
using namespace wspp::server ;

namespace wspp { namespace server {
namespace detail {

RequestParser::RequestParser()
{
    memset(&settings_, 0, sizeof(settings_));
    settings_.on_url = &on_url;

    settings_.on_message_complete = &on_message_complete;
    settings_.on_message_begin    = &on_message_begin;
    settings_.on_header_field     = &on_header_field;
    settings_.on_header_value     = &on_header_value;
    settings_.on_headers_complete = &on_headers_complete;
    settings_.on_body = &on_body;

    reset() ;
}

void RequestParser::reset()
{
    memset(&parser_, 0, sizeof(parser_));
    http_parser_init(&parser_, HTTP_REQUEST);

    parser_.data = this ;
    url_.clear() ;
    current_header_field_.clear() ;
    current_header_value_.clear() ;
    body_.clear() ;
}


int RequestParser::on_message_begin(http_parser *parser)
{
    RequestParser &rp = *static_cast<RequestParser*>(parser->data);
    rp.is_complete_ = false ;
    return 0;
}

int RequestParser::on_message_complete(http_parser * parser)
{
    RequestParser &rp = *static_cast<RequestParser*>(parser->data);
    rp.is_complete_ = true ;

    // Force the parser to stop after the Request is parsed so clients
    // can process the Request (or response).  This is to properly
    // handle HTTP/1.1 pipelined Requests.
    http_parser_pause(parser, 1);

    return 0;
}

int RequestParser::on_header_field(http_parser *parser, const char *data, size_t size)
{
    RequestParser &rp = *static_cast<RequestParser*>(parser->data);

    if ( !rp.current_header_value_.empty() ) {
        rp.headers_[rp.current_header_field_] = rp.current_header_value_ ;
        rp.current_header_field_.clear() ;
        rp.current_header_value_.clear() ;
    }
    rp.current_header_field_.append(data, size);
    return 0 ;
}

int RequestParser::on_header_value(http_parser *parser, const char *data, size_t size)
{
    RequestParser& rp = *static_cast<RequestParser*>(parser->data);
    rp.current_header_value_.append(data, size);
    return 0 ;
}

int RequestParser::on_headers_complete(http_parser * parser)
{
    RequestParser &rp = *static_cast<RequestParser*>(parser->data);

    if ( !rp.current_header_value_.empty() ) {
        rp.headers_[rp.current_header_field_] = rp.current_header_value_ ;
        rp.current_header_field_.clear() ;
        rp.current_header_value_.clear() ;
    }

    // Force the parser to stop after the headers are parsed so clients
    // can process the Request (or response).  This is to properly
    // handle HTTP/1.1 pipelined Requests.
    http_parser_pause(parser, 1);

    ostringstream strm ;
    strm << "HTTP/" << parser->http_major << "." << parser->http_minor ;
    rp.protocol_ = strm.str() ;

    return 0;
}

int RequestParser::on_url(http_parser *parser, const char *data, size_t size)
{
    RequestParser& rp = *static_cast<RequestParser*>(parser->data);
    rp.url_.assign(data, size);

    return 0;
}

int RequestParser::on_body(http_parser * parser, const char *data, size_t size)
{
    static_cast<RequestParser*>(parser->data)->body_.append(data, size) ;
    return 0;
}



boost::tribool RequestParser::parse(const char *data, size_t size)
{
    std::size_t used = http_parser_execute(&parser_, &settings_, data, size);

    const http_errno error = static_cast< http_errno >(parser_.http_errno);

  // The 'on_message_complete' and 'on_headers_complete' callbacks fail
  // on purpose to force the parser to stop between pipelined Requests.
  // This allows the clients to reliably detect the end of headers and
  // the end of the message.  Make sure the parser is always unpaused
  // for the next call to 'feed'.

  if (error == HPE_PAUSED) {
      http_parser_pause(&parser_, 0);
  }

  if (used < size)
  {
      if (error == HPE_PAUSED)
      {
          // Make sure the byte that triggered the pause
          // after the headers is properly processed.
          if ( !is_complete_ )
              used += http_parser_execute(&parser_, &settings_, data + used, size - used);
      }
      else {
          return false ;
      }
  }

  return ( is_complete_ ? boost::tribool(true) : boost::indeterminate ) ;
}

/////////////////////////////////////////////////////////////////////////////

static int hex_decode(char c)
{
    char ch = tolower(c) ;

    if ( ch >= 'a' && ch <= 'f' ) return 10 + ch - 'a' ;
    else if ( ch >= '0' && ch <= '9' ) return ch - '0' ;
    else return 0 ;
}

std::string url_decode(const char *str)
{
    const char *p = str ;

    std::string ret ;
    while ( *p )
    {
        if( *p == '+' ) ret += ' ' ;
        else if ( *p == '%' )
        {
            ++p ;
            char tmp[4];
            unsigned char val = 16 * ( hex_decode(*p++) )  ;
            val += hex_decode(*p) ;
            sprintf(tmp,"%c", val);
            ret += tmp ;
        } else ret += *p ;
        ++p ;
    }

    return ret;
}

static bool has_url_field( http_parser_url &url, http_parser_url_fields field )
{
    return ((url.field_set & (1 << int(field))) != 0);
}

static std::string get_url_field ( const string &data, http_parser_url &url, http_parser_url_fields field ) {
    if ( !has_url_field(url, field) ) return string() ;
    return ( data.substr(url.field_data[int(field)].off, url.field_data[int(field)].len) );
}

// fix invalid paths

static string normalize_path( const string &src ) {

    if ( src.empty() ) return "/" ;

    std::vector<std::string> src_segments, dst_segments;
    boost::split(src_segments, src, [](char ch) {
         return ch == '/';
    });

    for( const string &seg: src_segments ) {
        if ( seg.empty() ) continue ;
        else if ( seg == "." ) continue ;
        else if ( seg == "..") {
            if ( !dst_segments.empty() )
                dst_segments.pop_back() ;
        }
        else {
            dst_segments.push_back(seg) ;
        }
    }

    if ( dst_segments.empty() ) return "/" ;
    string res = "/" + boost::join(dst_segments, "/") ;
    return res ;
}

static bool parse_url(Request &req, const string url)
{
    http_parser_url u ;

    int result = http_parser_parse_url(url.c_str(), url.length(), 0, &u);

    if ( result ) return false ;

    string uri = get_url_field(url, u, UF_PATH) ;
    string query = get_url_field(url, u, UF_QUERY) ;

    req.path_ = normalize_path(url_decode(uri.c_str())) ;
    req.query_ = url_decode(query.c_str()) ;

    if ( !req.query_.empty() )
    {
        std::vector<std::string> args ;

        boost::split(args, req.query_, boost::is_any_of("&"), boost::algorithm::token_compress_on);

        for(int i=0 ; i<args.size() ; i++ )
        {
            std::string &arg = args[i] ;

            int pos = arg.find('=') ;

            if ( pos > 0 )
            {
                std::string key = arg.substr((int)0, (int)pos) ;
                std::string val = arg.substr((int)pos+1, -1) ;

                req.GET_[key] = ( val.empty() ) ? "" : val ;
            }
            else if ( pos == -1 )
                req.GET_[arg] = "" ;
        }
    }

    return true ;

}

static bool parse_cookie(Request &session, const std::string &data)
{
    int pos = data.find("=") ;

    if ( pos == -1 ) return false ;

    boost::regex rx("[ \n\r\t\f\v]*") ;
    boost::smatch rm ;

    boost::regex_search(data, rm, rx) ;
    int wscount = rm[0].length() ;

    std::string name = data.substr(wscount, pos - wscount);
    std::string value = data.substr(++pos);

    session.COOKIE_[name] = value ;

    return true ;
}

static bool parse_cookies(Request &session)
{
    const char *ck = "Cookie" ;

    if ( session.SERVER_.count(ck) == 0 ) return true ;

    std::string data = session.SERVER_[ck] ;

    if ( data.empty() ) return false ;

    int  old_pos = 0 ;

    while (1)
    {
        // find the ';' terminating a name=value pair
        int pos = data.find(";", old_pos);

        // if no ';' was found, the rest of the string is a single cookie

        if ( pos == -1 ) {
            bool res = parse_cookie(session, data.substr(old_pos, (int)-1));
            return res ;
        }

        // otherwise, the string contains multiple cookies
        // extract it and add the cookie to the list
        if ( !parse_cookie(session, data.substr(old_pos, pos - old_pos)) ) return false ;

        // update pos (+1 to skip ';')
        old_pos = pos + 1;
    }

    return true ;
}

static std::string get_next_line(std::istream &strm, int maxc = 1000)
{
    std::string res ;
    char b0, b1 ;
    int count = 0 ;

    while ( count < maxc && !strm.eof() )
    {
        b0 = strm.get() ;
        count ++ ;

        if ( b0 == '\r' )
        {
            b1 = strm.get() ;
            count ++ ;

            if ( b1 == '\n' ) return res ;
            else {
                res += b0 ;
                res += b1 ;
            }
        }
        else res += b0 ;

    }

    return res ;
}

namespace fs = boost::filesystem ;

fs::path get_temporary_path(const std::string &dir, const std::string &prefix, const std::string &ext)
{
    std::string retVal ;

    fs::path directory ;

    if ( ! dir.empty() ) directory = dir;
    else directory = boost::filesystem::temp_directory_path() ;

    std::string varname ="%%%%-%%%%-%%%%-%%%%";

    if ( !prefix.empty() )
        directory /= prefix + '-' + varname + '.' + ext ;
    else
        directory /= "tmp-" + varname + '.' + ext ;

    boost::filesystem::path temp = boost::filesystem::unique_path(directory);

    return temp;
}


static bool parse_mime_data(Request &session, istream &strm, const string &fld, const string &file_name,
                          const string &content_type,
                          const string trans_encoding,
                          const string &bnd)
{
    std::string data ;
    char b[4] = {0} ;

    while ( strm )
    {
        char c = strm.get();
        b[0] = b[1] ; b[1] = b[2] ; b[2] = b[3] ; b[3] = c ;

        if ( b[0] == '\r' && b[1] == '\n' && b[2] == '-' && b[3] == '-') {
            data.resize(data.size() - 3) ;
            int bndlen = bnd.length();
            string buf ;
            buf.resize(bndlen) ;

            strm.read(&buf[0], bndlen) ;

            if ( buf == bnd ) {
                strm.get() ; strm.get() ;
                break ;
            }
        }
        else data.push_back(c) ;
    }

    if ( file_name.empty() ) session.POST_[fld] = data ;
    else
    {
        static const size_t file_upload_max_size = 20 * 1024 * 1024 ;
        static const size_t file_upload_persist_file_size = 1 * 1024 * 1024 ;

        size_t dsize = data.size() ;

        if ( dsize <= file_upload_max_size ) {
            Request::UploadedFile file_info ;

            file_info.mime_ = content_type ;
            file_info.name_ = file_name ;
            file_info.data_ = data ;

/*
            if ( dsize > file_upload_persist_file_size ) {
                boost::filesystem::path server_path = get_temporary_path(string(), "up", "tmp") ;

                ofstream strm(server_path.string(), ios::binary) ;
                strm.write(&data[0], data.size()) ;

                file_info.path_ = server_path.string() ;
            } else {
            */

            /*}*/

            file_info.size_ = data.size() ;
            session.FILE_.insert({fld, file_info}) ;
        }
    }

    return true ;
}


static bool parse_multipart_data(Request &session, istream &strm, const string &bnd)
{
    std::string s = get_next_line(strm) ;
    if ( s.empty() ) return false ;

    if ( s.compare(2, bnd.length(), bnd ) != 0 ) return false ;

    while ( 1 )
    {

        std::string form_field, file_name, content_type, trans_encoding;

        while ( 1 )
        {
            s = get_next_line(strm) ;
            if ( s.empty() ) break ;

            size_t pos = s.find(':') ;

            if ( pos != string::npos ) {
                std::string key, val ;
                key.assign(s, 0, pos) ;
                boost::trim(key) ;
                val.assign(s, pos+1, s.length() - pos) ;
                boost::trim(val) ;

                boost::smatch subm ;
                if ( key == "Content-Disposition" ) {
                    if ( boost::regex_match(val, subm, boost::regex(R"#(form-data;\s*name="(.*?)(?=")"(?:\s*;\s*filename="(.*?)(?=")")?)#") ) )
                    {
                        form_field = subm[1] ;
                        file_name = subm[2] ;
                    }
                }
                else if ( key.compare(0, 12, "Content-Type") == 0 )
                    content_type = val ;
                else if ( key.compare(0, 25, "Content-Transfer-Encoding") == 0 )
                    trans_encoding = val ;
            }
        }

        if ( form_field.empty() ) break;

        // Parse content

        if ( ! parse_mime_data(session, strm, form_field, file_name, content_type, trans_encoding, bnd) ) return false ;


    }

    return true ;
}


static bool parse_form_data(Request &session, istream &strm)
{
    size_t content_length = session.SERVER_.value<int>("Content-Length", 0) ;

    std::string content_type = session.SERVER_.get("Content-Type") ;

    if ( content_type.empty() && content_length > 0 )
        return false ;

    boost::smatch subm ;

    if ( boost::starts_with(content_type, "application/x-www-form-urlencoded") )
    {
        // parse name value pairs

        std::string s = get_next_line(strm, content_length) ;

        typedef boost::tokenizer<boost::char_separator<char>>   tokenizer;

        boost::char_separator<char> sep("&");
        tokenizer tokens(s, sep);

        for (tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it)
        {
            std::string str = (*it) ;

            size_t pos = str.find('=') ;
            if ( pos == string::npos ) return false ;

            std::string key, val ;
            key = str.substr(0, pos) ;
            val = str.substr(pos+1) ;
            session.POST_[url_decode(key.c_str())] = url_decode(val.c_str()) ;
        }
    }
    else if ( boost::regex_match(content_type, subm, boost::regex("multipart/form-data;\\s*boundary=(.*)")  )) {
        std::string boundary = subm[1] ;
        parse_multipart_data(session, strm, boundary) ;

    }
    else if ( content_length )  {
        session.content_.resize(content_length) ;
        strm.read(&session.content_[0], content_length) ;
        session.content_type_ = content_type ;

    }

    return true ;
}


bool RequestParser::decode_message(Request &req) const {

    for( auto hdr: headers_ )
        req.SERVER_.add(hdr.first, hdr.second) ;

    req.method_ = req.SERVER_["REQUEST_METHOD"] =	http_method_str(static_cast<http_method>(parser_.method)) ;
    req.protocol_ = protocol_ ;
    req.content_ = body_ ;

    if ( !parse_url(req, url_) ||
        !parse_cookies(req) ) return false ;

    if ( req.method_ == "POST" )
    {
        stringstream reqstr(body_) ;

        if ( !parse_form_data(req, reqstr) ) return false ;
    }

    return true ;
}

} // namespace detail
} // namespace server
} // namespace wspp
