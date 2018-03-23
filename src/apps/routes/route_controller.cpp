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

RouteCreateForm::RouteCreateForm(const Request &req, RouteModel &routes): request_(req), routes_(routes) {

    field<InputField>("title", "text").label("Title").required()
        .addValidator<NonEmptyValidator>() ;

    field<SelectField>("mountain", std::make_shared<DictionaryOptionsModel>(routes_.getMountainsDict()))
    .required().label("Mountain") ;

    field<FileUploadField>("gpx-file").label("GPX file").required()
        .addValidator([&] (const string &val, const FormField &f) {

            auto it = request_.FILE_.find("gpx-file") ;
            if ( it == request_.FILE_.end() )
                throw FormFieldValidationError("No file received") ;

            const Request::UploadedFile &up = it->second ;

            GpxParser parser(up.data_, geom_) ;
            if ( !parser.parse() )
                throw FormFieldValidationError("Not valid GPX file") ;
    }) ;
}

void RouteCreateForm::onSuccess(const Request &request) {
    routes_.importRoute(getValue("title"), getValue("mountain"), geom_) ;
}

RouteUpdateForm::RouteUpdateForm(Connection &con, RouteModel &routes): con_(con), routes_(routes) {

    field<InputField>("title", "text").label("Title").required()
        .addValidator<NonEmptyValidator>() ;

    field<SelectField>("mountain", std::make_shared<DictionaryOptionsModel>(routes_.getMountainsDict()))
    .required().label("Mountain") ;
}

void RouteUpdateForm::onSuccess(const Request &request) {
    string id = request.POST_.get("id") ;
    routes_.updateInfo(id, getValue("title"), getValue("mountain")) ;
}

void RouteUpdateForm::onGet(const Request &request) {
    const Dictionary &params = request.GET_ ;
    string id = params.get("id") ;

    if ( id.empty() )
        throw HttpResponseException(Response::not_found) ;

    string title, mountain ;
    if ( !routes_.getInfo(id, title, mountain) )
        throw HttpResponseException(Response::not_found) ;

    init({{"title", title}, {"mountain", mountain}}) ;
}

class RouteTableView: public SQLTableView {
public:
    RouteTableView(Connection &con): SQLTableView(con, "routes_list_view")  {

        setTitle("Routes") ;

        con_.execute("CREATE TEMPORARY VIEW routes_list_view AS SELECT r.id as id, r.title as title, m.name as mountain FROM routes as r JOIN mountains as m ON m.id = r.mountain") ;

        addColumn("Title",  "<a href=\"route/edit/{{id}}\">{{title}}</a>") ;
        addColumn("Mountain", "{{mountain}}") ;
    }
};

void RouteController::fetch() {

    RouteTableView view(con_) ;
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page) ;

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
    string id = params.get("id") ;

    Statement stmt(con_, "UPDATE routes SET description = ? WHERE id = ?", content, id) ;
    stmt.exec() ;

    response_.writeJSONVariant(Variant::Object{{"id", id}, {"msg", "Route description succesfully updated"}}) ;
}


void RouteController::create() {
    RouteCreateForm form(request_, routes_) ;
    form.handle(request_, response_, engine_) ;
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

void RouteController::update() {
    RouteUpdateForm form(con_, routes_) ;

    form.handle(request_, response_, engine_) ;
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

    response_.encodeFileData(data, string(), mime, 0);
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

    if ( request_.matches("GET", "/")  ) {
        browse(std::string()) ;
        return true ;
    }
    if ( request_.matches("GET", "/mountain/{mountain:[\\w]+}?", attributes)  ) {
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
    else if ( request_.matches("POST", "/route/publish/") ) {
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

void RouteController::list() {
    RouteTableView view(con_) ;
    view.render(request_, response_, engine_) ;
}


