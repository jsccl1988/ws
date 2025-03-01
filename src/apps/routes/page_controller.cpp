#include "page_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std;
using namespace wspp::util;
using namespace wspp::web;
using namespace wspp::server;
using namespace wspp::db;

PageCreateForm::PageCreateForm(Connection &con): con_(con) {
    field("title").alias("Title").addValidator<NonEmptyValidator>();
    field("slug").alias("Slug").addValidator<RegexValidator>(boost::regex("[a-z0-9]+(?:-[a-z0-9]+)*"), "{field} can only contain alphanumeric words delimited by - ")
        .addValidator([&] (const string &val, const FormField &f) {
            bool error = con_.query("SELECT count(*) FROM pages WHERE permalink = ?", val).getOne()[0].as<int>();
            if ( error )
                throw FormFieldValidationError("A page with this slug already exists");
    });
}

void PageCreateForm::onSuccess(const Request &request) {
    // write data to database
    con_.execute("INSERT INTO pages ( title, permalink ) VALUES ( ?, ? )", getValue("title"), getValue("slug"));
}

PageUpdateForm::PageUpdateForm(Connection &con, const string &id): con_(con), id_(id) {
    field("title").alias("Title").addValidator<NonEmptyValidator>();
    field("slug").alias("Slug")
        .addValidator<RegexValidator>(boost::regex("[a-z0-9]+(?:-[a-z0-9]+)*"), "{field} can only contain alphanumeric words delimited by - ")
        .addValidator([&] (const string &val, const FormField &f) {
            bool error = con_.query("SELECT count(*) FROM pages WHERE permalink = ? AND id != ?", val, id_).getOne()[0].as<int>();
            if ( error )
                throw FormFieldValidationError("A page with this slug already exists");
    });
}

void PageUpdateForm::onSuccess(const Request &request) {
    con_.execute("UPDATE pages SET title = ?, permalink = ? WHERE id = ?",
                 getValue("title"), getValue("slug"), id_);
}

void PageUpdateForm::onGet(const Request &request) {
    const Dictionary &params = request.GET_;
    string id = params.get("id");

    if ( id.empty() )
        throw HttpResponseException(Response::not_found);

    QueryResult res = con_.query("SELECT title, permalink as slug FROM pages WHERE id = ? LIMIT 1", id);
    if ( res.next() )
        init(res.getAll());
}


class PageTableView: public SQLTableView {
public:
    PageTableView(Connection &con): SQLTableView(con, "pages_list_view" )  {
        con_.execute("CREATE TEMPORARY VIEW pages_list_view AS SELECT id, title, permalink as slug FROM pages");
    }
};

void PageController::fetch(){
    PageTableView view(con_);
    view.handle(request_, response_);
}
// CREATE TABLE pages (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, content TEXT, permalink TEXT);
void PageController::edit(){
    Variant::Object ctx{
        { "page", page_.data("edit_pages", "Edit pages") }
    };

    response_.write(engine_.render("pages-edit", ctx));
}

void PageController::publish(){
    const Dictionary &params = request_.POST_;
    string content = params.get("content");
    string permalink = params.get("slug");
    string id = params.get("id");

    con_.execute("UPDATE pages SET content = ? WHERE id = ?", content, id);
    string href = "/page/" + permalink;
    response_.writeJSONVariant(Variant::Object{{"id", id}, {"msg", "Page succesfully updated. View <a href=\"" + href + "\">page</a>"}});
}

void PageController::create(){
    PageCreateForm form(con_);
    form.handle(request_, response_, engine_);
}

void PageController::edit(const string &id){
    Query stmt(con_, "SELECT title, content, permalink FROM pages WHERE id=?");
    QueryResult res = stmt(id);

    if ( res.next() ) {
        string permalink, title, content;
        res >> title >> content >> permalink;

        Variant::Object ctx{
                     { "page", page_.data(permalink, title) },
                     { "id", id },
                     { "title", title },
                     { "content", content },
                     { "slug", permalink }
        };

        response_.write(engine_.render("page-edit", ctx));

    } else {
        throw HttpResponseException(Response::not_found);
    }
}

void PageController::update(){
    string id = request_.POST_.get("id");
    PageUpdateForm form(con_, id);
    form.handle(request_, response_, engine_);
}

void PageController::remove(){
    const Dictionary &params = request_.POST_;
    string id = params.get("id");

    if ( id.empty() )
        throw HttpResponseException(Response::not_found);
    else {
        con_.execute("DELETE FROM pages where id=?", id);
        response_.writeJSON("{}");
    }
}

bool PageController::dispatch(){
    if ( !boost::starts_with(request_.path_, "/page") )
        return false;

    Dictionary attributes;

    bool logged_in = user_.check();
    if ( request_.matches("GET", "/pages/edit/") ) { // load page list editor
        if ( logged_in && user_.can("pages.edit")) edit();
        else throw HttpResponseException(Response::unauthorized);
        return true;
    }

    if ( request_.matches("GET", "/pages/list/") ) { // fetch table data
        if ( logged_in ) fetch();
        else throw HttpResponseException(Response::unauthorized);
        return true;
    }

    if ( request_.matches("GET|POST", "/pages/add/") ) {
        if ( logged_in ) create();
        else throw HttpResponseException(Response::unauthorized);
        return true;
    }
    if ( request_.matches("GET|POST", "/pages/update/") ) {
        if ( logged_in ) update();
        else throw HttpResponseException(Response::unauthorized);
        return true;
    } else if ( request_.matches("GET", "/page/edit/{id}/", attributes) ) {
        if ( logged_in ) edit(attributes.get("id"));
        else throw HttpResponseException(Response::unauthorized);
        return true;
    } else if ( request_.matches("POST", "/page/publish/") ) {
        if ( logged_in ) publish();
        else throw HttpResponseException(Response::unauthorized);
        return true;
    } else if ( request_.matches("POST", "/pages/delete/") ) {
        if ( logged_in ) remove();
        else throw HttpResponseException(Response::unauthorized);
        return true;
    } else if ( request_.matches("GET", "/page/{id}/", attributes) ) {
        show(attributes.get("id"));
        return true;
    } else
        return false;
}

void PageController::show(const std::string &page_id){
    QueryResult res = con_.query("SELECT id, title, content FROM pages WHERE permalink=?", page_id);
    if ( res.next() ) {
        Variant::Object ctx{
                     { "page", page_.data(page_id, res.get<string>("title")) },
                     { "content", res.get<string>("content") },
                     { "id", res.get<int>("id") }
        };

        response_.write(engine_.render("page", ctx));
    } else {
        throw HttpResponseException(Response::not_found);
    }
}