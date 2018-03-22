#ifndef __WSPP_SERVER_FS_SESSION_HANDLER_HPP__
#define __WSPP_SERVER_FS_SESSION_HANDLER_HPP__

#include <wspp/server/session_handler.hpp>
#include <wspp/database/connection.hpp>

#include <string>

namespace wspp { namespace server {

using util::Dictionary ;

// SQlite3 storage of session data

class FileSystemSessionHandler: public SessionHandler {
public:
    FileSystemSessionHandler(const std::string &db_file = std::string()) ;

private:

    virtual bool open() override ;
    virtual bool close() override ;
    virtual bool write(const Session &session) override ;
    virtual bool read(Session &session) override ;
    std::string uniqueSID() override ;

private:

    std::string serializeData(const Dictionary &data) ;
    void deserializeData(const std::string &data, Dictionary &cont) ;

    bool writeSessionData(const std::string &id, const std::string &data) ;
    bool readSessionData(const std::string &id, std::string &data) ;

    bool contains(const std::string &id) ;

    void gc() ;

    db::Connection db_ ;
};


} // namespace server
} // namespace wspp

#endif
