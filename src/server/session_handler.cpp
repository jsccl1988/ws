#include <wspp/server/session_handler.hpp>
#include <wspp/util/random.hpp>

using namespace std ;

namespace wspp {

string SessionHandler::generateSID() {
    return binToHex(randomBytes(32)) ;
}

/*
void SessionManager::open(const Request &req, Session &session_data)
{
    session_data.id_ = req.COOKIE_.get("WSX_SESSION_ID") ;

    if ( session_data.id_.empty() ) {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        session_data.id_ = boost::lexical_cast<std::string>(uuid) ;
    }

    load(session_data) ;
}

void SessionManager::close(Response &resp, const Session &session_data) {

    save(session_data) ;
    resp.headers_.add("Set-Cookie", "WSX_SESSION_ID=" + session_data.id_) ;
}


void MemSessionManager::save(const Session &session)  { data_[session.id_] = session.data_ ;}
void MemSessionManager::load(Session &session) { session.data_ = data_[session.id_] ; }

*/

}
