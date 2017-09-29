#include "wpts_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

WaypointUpdateForm::WaypointUpdateForm(sqlite::Connection &con, const RouteModel &routes): con_(con), routes_(routes) {

    field<InputField>("name", "text").label("Name").required()
        .addValidator<NonEmptyValidator>() ;

    field<InputField>("desc", "text").label("Description") ;
}

class WaypointTableView: public SQLiteTableView {
public:
    WaypointTableView(Connection &con, const std::string &route_id):
        SQLiteTableView(con, "wpt_list_view") {

        string sql("CREATE TEMPORARY VIEW wpt_list_view AS SELECT id, name, desc, ST_X(geom) as lon, ST_Y(geom) as lat, ele FROM wpts WHERE route = ") ;
        sql += route_id;

        sqlite::Statement stmt(con_, sql) ;
        stmt.exec() ;

        addColumn("Name", "{{name}}") ;
        addColumn("Description", "{{desc}}") ;
        addColumn("Location", "{{> show-wpt-on-map}}" ) ;
    }

};

void WaypointController::update(const std::string &route_id)
{
    if ( request_.method_ == "POST" ) {

        string id = request_.POST_.get("id") ;

        WaypointUpdateForm form(con_, routes_) ;

        if ( form.validate(request_) ) {

            // write data to database

            routes_.updateWaypoint(id, form.getValue("name"), form.getValue("desc")) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("waypoint-edit-dialog", ctx)}});
        }
    }
    else {

        const Dictionary &params = request_.GET_ ;
        string id = params.get("id") ;

        WaypointUpdateForm form(con_, routes_) ;

        if ( id.empty() ) {
            throw HttpResponseException(Response::not_found) ;
        }

        string name, desc ;
        if ( !routes_.getWaypoint(id, name, desc) ) {
            throw HttpResponseException(Response::not_found) ;
        }

        form.init({{"name", name}, {"desc", desc}}) ;

        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("waypoint-edit-dialog", ctx)) ;
    }

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

    if ( request_.matches("GET", "/wpts/list/{id:\\d+}", attributes) ) {
        if ( logged_in ) list(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/wpts/update/{id:\\d+}", attributes) ) {
        if ( logged_in ) update(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/wpts/delete/{id:\\d+}", attributes) ) {
        if ( logged_in ) remove(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    } else return false ;
}

void WaypointController::list(const std::string &route_id)
{
    WaypointTableView view(con_, route_id) ;
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page) ;

    response_.write(engine_.render("waypoints-table-view", data )) ;
}

