//
// Connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

//#include <wspp/server/detail/connection.hpp>
/*
#include <wspp/server/detail/request_handler_factory.hpp>
#include <wspp/server/detail/connection_manager.hpp>
#include <wspp/server/session_manager.hpp>
#include <wspp/server/request_context.hpp>

#include <vector>
#include <boost/bind.hpp>
*/
namespace wspp {
/*
Connection::Connection(boost::asio::ip::tcp::socket socket,
                       ConnectionManager& manager,
                       SessionManager &sm,
                       boost::shared_ptr<RequestHandler> handler)
    : socket_(std::move(socket)), ConnectionBase(manager, sm, handler)
{
}

extern std::vector<boost::asio::const_buffer> response_to_buffers(const Response &rep) ;

void Connection::read() {


    socket_.async_read_some(boost::asio::buffer(buffer_), [&] (boost::system::error_code e, std::size_t bytes_transferred) {
        if (!e)
        {
            boost::tribool result;
            result = request_parser_.parse(buffer_.data(), bytes_transferred);

            if ( result )
            {
                if ( !request_parser_.decode_message(request_) ) {
                    reply_.stock_reply(Response::bad_request);
                }
                else {

                    if ( handler_ ) {

                        Session session ;
                        session_manager_.open(request_, session) ;

                        try {
                            handler_->handle(request_, reply_, session) ;
                            session_manager_.close(reply_, session) ;
                        }
                        catch ( ... ) {
                            reply_.stock_reply(Response::internal_server_error);
                        }

                    }
                    else
                        reply_.stock_reply(Response::not_found);

                }

                write(response_to_buffers(reply_)) ;

            }
            else if (!result)
            {
                reply_.stock_reply(Response::bad_request);

                write(response_to_buffers(reply_)) ;

            }
            else
            {
                read() ;
            }
        }
        else if (e != boost::asio::error::operation_aborted)
        {
            stop() ;
        }

    });
}

void Connection::write(const std::vector<boost::asio::const_buffer> &buffers) {

    boost::asio::async_write(socket_, buffers, [this](boost::system::error_code e, std::size_t) {
        if (!e)
        {
            // Initiate graceful Connection closure.
            boost::system::error_code ignored_ec;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        }

        if (e != boost::asio::error::operation_aborted)
        {
            stop() ;
        }
   });
}


void Connection::stop() {
    boost::shared_ptr<Connection> con(this) ;
    connection_manager_.stop(con);
    socket_.close();
}

*/

} // namespace wspp
