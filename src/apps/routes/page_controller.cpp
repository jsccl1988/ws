#include "page_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

PageEditForm::PageEditForm(sqlite::Connection &con, const string &id): con_(con), id_(id) {

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


class PageTableView: public SQLiteTableView {
public:
    PageTableView(Connection &con): SQLiteTableView(con, "pages_list_view" )  {

        con_.exec("CREATE TEMPORARY VIEW pages_list_view AS SELECT id, title, permalink as slug FROM pages") ;

        addColumn("Title", "{{title}}") ;
        addColumn("Slug", "{{slug}}") ;
    }
};

void PageController::fetch()
{
    PageTableView view(con_) ;
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page ) ;

    response_.write(engine_.render("pages-table-view", data )) ;
}
// CREATE TABLE pages (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, content TEXT, permalink TEXT);

void PageController::edit()
{
    Variant ctx( Variant::Object{
                 { "page", page_.data("edit_pages", "Edit pages") }
    }) ;

    response_.write(engine_.render("pages-edit", ctx)) ;
}

void PageController::publish()
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

void PageController::create()
{
    PageEditForm form(con_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_) ) {

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

void PageController::edit(const string &id)
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

void PageController::update()
{
    if ( request_.method_ == "POST" ) {

        string id = request_.POST_.get("id") ;

        PageEditForm form(con_, id) ;

        if ( form.validate(request_) ) {

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

        PageEditForm form(con_, id) ;

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

void PageController::remove()
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

bool PageController::dispatch()
{
    if ( !boost::starts_with(request_.path_, "/page") ) return false ;

    Dictionary attributes ;

    bool logged_in = user_.check() ;

    if ( request_.matches("GET", "/pages/edit/") ) { // load page list editor
        if ( logged_in && user_.can("pages.edit")) edit() ;
        else throw HttpResponseException(Response::unauthorized) ;
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
    else if ( request_.matches("GET", "/page/{id}/", attributes) ) {
        show(attributes.get("id")) ;
        return true ;
    }
    else
        return false ;
}

void PageController::show(const std::string &page_id)
{
    sqlite::Query q(con_, "SELECT id, title, content FROM pages WHERE permalink=?", page_id) ;
    sqlite::QueryResult res = q.exec() ;

    if ( res ) {

        Variant ctx( Variant::Object{
                     { "page", page_.data(page_id, res.get<string>("title")) },
                     { "content", res.get<string>("content") },
                     { "id", res.get<int>("id") }
        }) ;

        response_.write(engine_.render("page", ctx)) ;
    }
    else
        throw HttpResponseException(Response::not_found) ;

}


