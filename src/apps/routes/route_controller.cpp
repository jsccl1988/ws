#include "route_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

RouteEditForm::RouteEditForm(sqlite::Connection &con, const string &id): con_(con), id_(id) {

    field<InputField>("title", "text").label("Title").required()
        .addValidator<NonEmptyValidator>() ;

    field<InputField>("slug", "text").label("Slug").required()
        .addValidator<RegexValidator>(boost::regex("[a-z0-9]+(?:-[a-z0-9]+)*"), "{field} can only contain alphanumeric words delimited by - ")
        .addValidator([&] (const string &val, const FormField &f) {
            bool error ;
            if ( id_.empty() ) {
                sqlite::Query q(con_, "SELECT count(*) FROM pages WHERE permalink = ?") ;
                sqlite::QueryResult res = q(val) ;
                error = res.get<int>(0) ;
            }
            else {
                sqlite::Query q(con_, "SELECT count(*) FROM pages WHERE permalink = ? AND id != ?", val, id_) ;
                sqlite::QueryResult res = q.exec() ;
                error = res.get<int>(0) ;
            }

            if ( error ) {
                throw FormFieldValidationError("A page with this slug already exists") ;
            }
        }) ;
}


class RouteTableView: public SQLiteTableView {
public:
    RouteTableView(Connection &con): SQLiteTableView(con, "pages_list_view")  {

        con_.exec("CREATE TEMPORARY VIEW pages_list_view AS SELECT id, title, permalink as slug FROM pages") ;

        addColumn("Title", "title") ;
        addColumn("Slug", "slug") ;
    }
};

void RouteController::fetch()
{
    RouteTableView view(con_) ;
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page ) ;

    response_.write(engine_.render("pages-table-view", data )) ;
}
// CREATE TABLE pages (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, content TEXT, permalink TEXT);

void RouteController::edit()
{
    Variant ctx( Variant::Object{
                 { "page", page_.data("edit_pages", "Edit pages") }
    }) ;

    response_.write(engine_.render("pages-edit", ctx)) ;
}

void RouteController::publish()
{
    const Dictionary &params = request_.POST_ ;
    string content = params.get("content") ;
    string permalink = params.get("slug") ;
    string id = params.get("id") ;

    sqlite::Statement stmt(con_, "UPDATE pages SET content = ? WHERE id = ?", content, id) ;
    stmt.exec() ;
    string href = "/page/" + permalink ;
    response_.writeJSONVariant(Variant::Object{{"id", id}, {"msg", "Page succesfully updated. View <a href=\"" + href + "\">page</a>"}}) ;

}

void RouteController::create()
{
    RouteEditForm form(con_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_.POST_) ) {

            // write data to database

            sqlite::Statement stmt(con_, "INSERT INTO pages ( title, permalink ) VALUES ( ?, ? )") ;

            stmt.bind(1, form.getValue("title")) ;
            stmt.bind(2, form.getValue("slug")) ;
            stmt.exec() ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("page-edit-dialog-new", ctx)}});
        }
    }
    else {
        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("page-edit-dialog-new", ctx)) ;
    }

}

void RouteController::edit(const string &id)
{

    sqlite::Query stmt(con_, "SELECT title, content, permalink FROM pages WHERE id=?") ;
    sqlite::QueryResult res = stmt(id) ;

    if ( res ) {

        string permalink, title, content ;
        res.into(title, content, permalink) ;

        Variant ctx( Variant::Object{
                     { "page", page_.data(permalink, title) },
                     { "id", id },
                     { "title", title },
                     { "content", content },
                     { "slug", permalink }
        }) ;

        response_.write(engine_.render("page-edit", ctx)) ;

    }
    else
        throw HttpResponseException(Response::not_found) ;
}

void RouteController::update()
{
    if ( request_.method_ == "POST" ) {

        string id = request_.POST_.get("id") ;

        RouteEditForm form(con_, id) ;

        if ( form.validate(request_.POST_) ) {

            // write data to database

            sqlite::Statement stmt(con_, "UPDATE pages SET title = ?, permalink = ? WHERE id = ?") ;

            stmt(form.getValue("title"), form.getValue("slug"), id) ;


            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("page-edit-dialog-new", ctx)}});
        }
    }
    else {

        const Dictionary &params = request_.GET_ ;
        string id = params.get("id") ;

        RouteEditForm form(con_, id) ;

        if ( id.empty() ) {
            throw HttpResponseException(Response::not_found) ;
        }

        sqlite::Query q(con_, "SELECT title, permalink as slug FROM pages WHERE id = ? LIMIT 1", id) ;
        sqlite::QueryResult res = q.exec() ;

        if ( !res ) {
            response_.stock_reply(Response::not_found) ;
            return ;
        }

        form.init(res.getAll()) ;

        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("page-edit-dialog-new", ctx)) ;
    }

}

void RouteController::track(const string &id) {
    Variant data = routes_.fetchTrackGeoJSON(id) ;
    response_.writeJSONVariant(data) ;
}

static string url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for ( char c : value ) {
        if ( isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

void RouteController::download(const string &format, const string &route_id) {
    vector<Track> tracks ;
    vector<Waypoint> wpts ;
    routes_.fetchTracks(route_id, tracks, wpts) ;

    string mime, data ;
    if ( format == "gpx" ) {
        mime = "application/gpx+xml" ;
        data = RouteModel::exportGpx(tracks, wpts) ;
    } else {
        mime = "application/vnd.google-earth.kml+xml" ;
        data = RouteModel::exportKml(tracks, wpts) ;
    }

    string title = routes_.fetchTitle(route_id) ;

    // Output headers.

    string file_name = title + '.' + format ;

    response_.headers_.replace("Cache-Control", "private");
    if ( request_.SERVER_.get("HTTP_USER_AGENT").find("MSIE") != string::npos ) {
        response_.headers_.replace("Content-Disposition", "attachment; filename=" + url_encode ( file_name )) ;
    }  else {
        response_.headers_.replace("Content-Disposition", "attachment; filename*=UTF-8''" + url_encode ( file_name )) ;
    }

    response_.encode_file_data(data, string(), mime, 0);
}

void RouteController::remove()
{
   const Dictionary &params = request_.POST_ ;
    string id = params.get("id") ;

    if ( id.empty() )
        throw HttpResponseException(Response::not_found) ;
    else {
        sqlite::Statement stmt(con_, "DELETE FROM pages where id=?", id) ;
        stmt.exec() ;
        response_.writeJSON("{}") ;
    }

}

bool RouteController::dispatch()
{
    Dictionary attributes ;

    bool logged_in = user_.check() ;

    if ( request_.matches("GET", "/{mountain:[\\w]+}?", attributes)  ) {
        list(attributes.get("mountain")) ;
        return true ;
    }
    if ( request_.matches("GET", "/pages/list/") ) { // fetch table data
        if ( logged_in ) fetch() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/pages/add/") ) {
        if ( logged_in ) create() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/pages/update/") ) {
        if ( logged_in ) update() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("GET", "/page/edit/{id}/", attributes) ) {
        if ( logged_in ) edit(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/page/publish/") ) {
        if ( logged_in ) publish() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/page/delete/") ) {
        if ( logged_in ) remove() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("GET", "/download/track/{format:gpx|kml}/{id}", attributes) ) {
        download(attributes.get("format"), attributes.get("id")) ;
        return true ;
    }
    else if ( request_.matches("GET", "/track/{id}/", attributes) ) {
        track(attributes.get("id")) ;
        return true ;
    }
    else if ( request_.matches("GET", "/view/{id}/", attributes) ) {
        view(attributes.get("id")) ;
        return true ;
    }
    else
        return false ;
}

void RouteController::view(const std::string &route_id) {

    Variant route = routes_.fetch(route_id) ;

    if ( route.isNull() )
        throw HttpResponseException(Response::not_found) ;
    else {
        Variant attachments = routes_.fetchAttachments(route_id) ;
        Variant ctx( Variant::Object{
             { "page", page_.data("view", route.at("title").toString()) },
             { "route", route },
             { "attachments", attachments },
             { "id", route_id }
        }) ;

        response_.write(engine_.render("route-view", ctx)) ;
    }

}

void RouteController::list(const string &mountain)
{
    if ( !mountain.empty() ) {

        Variant routes = routes_.fetchMountain(mountain) ;

        string mname = routes_.getMountainName(mountain) ;
        Variant ctx( Variant::Object{
                     { "page", page_.data("routes", mname) },
                     { "name", mname },
                     { "routes", routes },
                 } )  ;
        response_.write(engine_.render("routes-mountain", ctx)) ;
    } else {
        Variant routes = routes_.fetchAllByMountain() ;

        Variant ctx( Variant::Object{
                     { "page", page_.data("routes", "Όλες οι διαδρομές") },
                     { "mountains", routes },
                 } )  ;
        response_.write(engine_.render("routes-all", ctx)) ;
    }


}


