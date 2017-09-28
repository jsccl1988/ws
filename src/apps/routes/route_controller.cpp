#include "route_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

#include "gpx_parser.hpp"

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

RouteCreateForm::RouteCreateForm(const Request &req, const RouteModel &routes): request_(req), routes_(routes) {

    field<InputField>("title", "text").label("Title").required()
        .addValidator<NonEmptyValidator>() ;

    field<SelectField>("mountain", boost::make_shared<DictionaryOptionsModel>(routes_.getMountainsDict()))
    .required().label("Mountain") ;

    field<FileUploadField>("gpx-file").label("GPX file").required()
        .addValidator([&] (const string &val, const FormField &f) {

            auto it = request_.FILE_.find("gpx-file") ;
            if ( it == request_.FILE_.end() )
                throw FormFieldValidationError("No file received") ;

            const Request::UploadedFile &up = it->second ;

            istringstream strm(up.data_) ;

            GpxParser parser(strm, geom_) ;
            if ( !parser.parse() )
                throw FormFieldValidationError("Not valid GPX file") ;
        }) ;
}



RouteUpdateForm::RouteUpdateForm(sqlite::Connection &con, const RouteModel &routes): con_(con), routes_(routes) {

    field<InputField>("title", "text").label("Title").required()
        .addValidator<NonEmptyValidator>() ;

    field<SelectField>("mountain", boost::make_shared<DictionaryOptionsModel>(routes_.getMountainsDict()))
    .required().label("Mountain") ;
}


class RouteTableView: public SQLiteTableView {
public:
    RouteTableView(Connection &con): SQLiteTableView(con, "routes_list_view")  {

        con_.exec("CREATE TEMPORARY VIEW routes_list_view AS SELECT r.id as id, r.title as title, m.name as mountain FROM routes as r JOIN mountains as m ON m.id = r.mountain") ;

        addColumn("Title", "title", "<a href=\"route/edit/{{id}}\">{{value}}</a>") ;
        addColumn("Mountain", "mountain") ;
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

void RouteController::query() {
    double x = request_.POST_.value<double>("x", 0) ;
    double y = request_.POST_.value<double>("y", 0) ;
    response_.writeJSONVariant(routes_.query(x, y)) ;
}
// CREATE TABLE pages (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, content TEXT, permalink TEXT);

void RouteController::edit()
{
    Variant ctx( Variant::Object{
        { "page", page_.data("edit_routes", "Edit routes") }
    }) ;

    response_.write(engine_.render("routes-edit", ctx)) ;
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
    RouteCreateForm form(request_, routes_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_) ) {

            routes_.importRoute(form.getValue("title"), form.getValue("mountain"), form.geom()) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("route-create-dialog", ctx)}});
        }
    }
    else {
        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("route-create-dialog", ctx)) ;
    }

}

void RouteController::edit(const string &id)
{
    Variant data = routes_.fetch(id) ;

    if ( data.isNull() )
        throw HttpResponseException(Response::not_found) ;

    Variant wpts = routes_.fetchWaypoints(id) ;
    Variant attachments = routes_.fetchAttachments(id) ;

    Variant ctx( Variant::Object{
        { "page", page_.data("edit_route", "Edit route") },
        { "route", data },
        { "wpts", wpts },
        { "attachments", attachments }
    }) ;

    response_.write(engine_.render("route-edit", ctx)) ;
}

void RouteController::update()
{
    if ( request_.method_ == "POST" ) {

        string id = request_.POST_.get("id") ;

        RouteUpdateForm form(con_, routes_) ;

        if ( form.validate(request_) ) {

            // write data to database

            routes_.updateInfo(id, form.getValue("title"), form.getValue("mountain")) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("route-edit-dialog", ctx)}});
        }
    }
    else {

        const Dictionary &params = request_.GET_ ;
        string id = params.get("id") ;

        RouteUpdateForm form(con_, routes_) ;

        if ( id.empty() ) {
            throw HttpResponseException(Response::not_found) ;
        }

        string title, mountain ;
        if ( !routes_.getInfo(id, title, mountain) ) {
            throw HttpResponseException(Response::not_found) ;
        }

        form.init({{"title", title}, {"mountain", mountain}}) ;

        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("route-edit-dialog", ctx)) ;
    }

}


void RouteController::track(const string &id) {
    RouteGeometry geom ;
    routes_.fetchGeometry(id, geom);
    Variant data = RouteModel::exportGeoJSON(geom) ;
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
    RouteGeometry geom ;
    routes_.fetchGeometry(route_id, geom) ;

    string mime, data ;
    if ( format == "gpx" ) {
        mime = "application/gpx+xml" ;
        data = RouteModel::exportGpx(geom) ;
    } else {
        mime = "application/vnd.google-earth.kml+xml" ;
        data = RouteModel::exportKml(geom) ;
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
        if ( routes_.remove(id) )
            response_.writeJSON("{}") ;
        else
            throw HttpResponseException(Response::not_found) ;
    }
}

bool RouteController::dispatch()
{
    Dictionary attributes ;

    bool logged_in = user_.check() ;

    if ( request_.matches("GET", "/{mountain:[\\w]+}?", attributes)  ) {
        browse(attributes.get("mountain")) ;
        return true ;
    }
    if ( request_.matches("GET", "/routes/edit/") ) {
        if ( logged_in ) edit() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET", "/routes/list/") ) {
        if ( logged_in ) list() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/routes/add/") ) {
        if ( logged_in ) create() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/routes/update/") ) {
        if ( logged_in ) update() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("GET", "/route/edit/{id}/", attributes) ) {
        if ( logged_in ) edit(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/page/publish/") ) {
        if ( logged_in ) publish() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/routes/delete/") ) {
        if ( logged_in ) remove() ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/query/route") ) {
        query() ;
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

void RouteController::browse(const string &mountain)
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

void RouteController::list()
{
    RouteTableView view(con_) ;
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page) ;

    response_.write(engine_.render("routes-table-view", data )) ;
}

