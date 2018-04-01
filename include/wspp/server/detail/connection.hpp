//
// HttpConnection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_CONNECTION_HPP
#define HTTP_SERVER_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <wspp/server/response.hpp>
#include <wspp/server/request.hpp>
#include <wspp/util/logger.hpp>

#include <wspp/server/request_handler.hpp>
#include <wspp/server/filter_chain.hpp>
#include <wspp/server/exceptions.hpp>
#include <wspp/server/detail/request_parser.hpp>
#include <wspp/server/detail/connection_manager.hpp>

namespace wspp { namespace server {

class ConnectionManager ;
class Server ;


using util::Logger ;

extern std::vector<boost::asio::const_buffer> response_to_buffers(Response &rep, bool) ;

/// Represents a single HttpConnection from a client.

class HttpConnection:
        public boost::enable_shared_from_this<HttpConnection>
{
public:
    explicit HttpConnection(boost::asio::ip::tcp::socket socket,
                        ConnectionManager& manager,
                        FilterChain &handler) : socket_(std::move(socket)),
        connection_manager_(manager), handler_(handler) {}

private:

    friend class Server ;
    friend class ConnectionManager ;



    void start() {
        read() ;
    }
    void stop() {
        socket_.close();
    }


    void read() {
        auto self(this->shared_from_this());
        socket_.async_read_some(boost::asio::buffer(buffer_), [self, this] (boost::system::error_code e, std::size_t bytes_transferred) {
            if (!e)
            {
                boost::tribool result;
                result = request_parser_.parse(buffer_.data(), bytes_transferred);

                if ( result )
                {
                    if ( !request_parser_.decode_message(request_) ) {
                        response_.stockReply(Response::bad_request);
                    }
                    else {
                        request_.SERVER_.add("REMOTE_ADDR", socket_.remote_endpoint().address().to_string() ) ;

                         try {
                             handler_.handle(request_, response_) ;

                             if ( response_.status_ != Response::ok )
                                 response_.stockReply(response_.status_);
                         }
                         catch ( HttpResponseException &e  ) {

                            response_.status_ = e.code_ ;
                            if ( e.reason_.empty() )
                                response_.stockReply(e.code_);
                            else {
                                response_.content_.assign(e.reason_);
                                response_.setContentType("text/html");
                                response_.setContentLength() ;
                            }

                         }

                        catch ( std::runtime_error &e ) {
                            std::cout << e.what() << std::endl ;
                            response_.stockReply(Response::internal_server_error) ;
                        }
                    }

                    write(response_to_buffers(response_, request_.method_ == "HEAD")) ;

                }
                else if (!result)
                {
                    response_.stockReply(Response::bad_request);

                    write(response_to_buffers(response_, request_.method_ == "HEAD")) ;

                }
                else
                {
                    read() ;
                }
            }
            else if (e != boost::asio::error::operation_aborted)
            {
                connection_manager_.stop(self) ;
            }

        });
    }

    void write(const std::vector<boost::asio::const_buffer> &buffers)  {
        auto self(this->shared_from_this());
        boost::asio::async_write(socket_, buffers, [this, self](boost::system::error_code e, std::size_t) {
            if (!e)
            {
                // Initiate graceful Connection closure.
                boost::system::error_code ignored_ec;
                socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
            }

            if (e != boost::asio::error::operation_aborted)
            {
                connection_manager_.stop(self) ;
            }
       });
    }

     boost::asio::ip::tcp::socket socket_;


     /// The handler of incoming HttpRequest.
     FilterChain &handler_;

     /// Buffer for incoming data.
     boost::array<char, 8192> buffer_;

     ConnectionManager& connection_manager_ ;

      /// The parser for the incoming HttpRequest.
     detail::RequestParser request_parser_;

     Request request_ ;
     Response response_ ;

};


} // namespace server
} // namespace wspp

#endif
