#include "wpts_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;
using namespace wspp::db ;

class WaypointUpdateForm: public wspp::web::FormHandler {
public:
    WaypointUpdateForm(RouteModel &routes, const std::string &route_id): routes_(routes), route_id_(route_id) {
        field("name").alias("Name")
            .addValidator<NonEmptyValidator>() ;

        field("desc").alias("Description") ;
    }

    void onSuccess(const wspp::server::Request &request) override {
        string id = request.POST_.get("id") ;
        routes_.updateWaypoint(id, getValue("name"), getValue("desc")) ;
    }
    void onGet(const wspp::server::Request &request) override {
        const Dictionary &params = request.GET_ ;
        string id = params.get("id") ;

        if ( id.empty() )
            throw HttpResponseException(Response::not_found) ;

        string name, desc ;
        if ( !routes_.getWaypoint(id, name, desc) ) {
            throw HttpResponseException(Response::not_found) ;
        }

        init({{"name", name}, {"desc", desc}}) ;
    }

private:

    RouteModel &routes_ ;
    string route_id_ ;
};


class WaypointTableView: public SQLTableView {
public:
    WaypointTableView(Connection &con, const std::string &route_id):
        SQLTableView(con, "wpt_list_view") {

        string sql("CREATE TEMPORARY VIEW wpt_list_view AS SELECT id, name, desc, ST_X(geom) as lon, ST_Y(geom) as lat, ele FROM wpts WHERE route = ") ;
        sql += route_id;

        con_.execute(sql) ;
    }

};

void WaypointController::update(const std::string &route_id)
{
    WaypointUpdateForm form(routes_, route_id) ;

    form.handle(request_, response_, engine_) ;
}

void WaypointController::remove(const std::string &route_id)
{
   const Dictionary &params = request_.POST_ ;
   string id = params.get("id") ;

    if ( id.empty() )
        throw HttpResponseException(Response::not_found) ;
    else {
        if ( routes_.removeWaypoint(id) )
            response_.writeJSON("{}") ;
        else
            throw HttpResponseException(Response::not_found) ;
    }
}

bool WaypointController::dispatch()
{
    Dictionary attributes ;

    bool logged_in = user_.check() ;

    if ( request_.matches("GET", "/wpts/{id:\\d+}/list", attributes) ) {
        if ( logged_in ) list(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/wpts/{id:\\d+}/update", attributes) ) {
        if ( logged_in ) update(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/wpts/{id:\\d+}/delete", attributes) ) {
        if ( logged_in ) remove(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    } else return false ;
}

void WaypointController::list(const std::string &route_id)
{
    WaypointTableView view(con_, route_id) ;
    view.handle(request_, response_) ;
}

