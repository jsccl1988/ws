#ifndef __SERVER_CONNECTION_MANAGER_HPP__
#define __SERVER_CONNECTION_MANAGER_HPP__

#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace wspp {
namespace server {
class HttpConnection;
typedef boost::shared_ptr<HttpConnection> ConnectionPtr;

// Manages open connections so that they may be cleanly stopped when the server
// needs to shut down.
class ConnectionManager{
public:
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    // Construct a connection manager.
    ConnectionManager();

    // Add the specified connection to the manager and start it.
    void start(ConnectionPtr c);

    // Stop the specified connection.
    void stop(ConnectionPtr c);

    // Stop all connections.
    void stop_all();

private:
    // The managed connections.
    std::set<ConnectionPtr> connections_;
    boost::mutex mutex_;
};
} // namespace server
} // namespace wspp
#endif

