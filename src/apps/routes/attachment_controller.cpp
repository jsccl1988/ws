#include "attachment_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

AttachmentCreateForm::AttachmentCreateForm(const Request &req, const RouteModel &routes): request_(req), routes_(routes) {

    field<SelectField>("type", boost::make_shared<DictionaryOptionsModel>(routes_.getAttachmentsDict()))
    .required().label("Type") ;

    const size_t max_attachment_file_size = 5 * 1024 * 1024;

    field<FileUploadField>("attachment-file").maxFileSize(max_attachment_file_size).label("Attachment").required()
        .addValidator([&] (const string &val, const FormField &f) {
            auto it = request_.FILE_.find("attachment-file") ;
            if ( it == request_.FILE_.end() )
                throw FormFieldValidationError("No file received") ;
        }) ;
}


AttachmentUpdateForm::AttachmentUpdateForm(sqlite::Connection &con, const RouteModel &routes): con_(con), routes_(routes) {

    field<InputField>("name", "text").label("Name").required()
        .addValidator<NonEmptyValidator>() ;

    field<SelectField>("type", boost::make_shared<DictionaryOptionsModel>(routes_.getAttachmentsDict()))
    .required().label("Type") ;
}


class AttachmentTableView: public SQLiteTableView {
public:
    AttachmentTableView(Connection &con, const std::string &route_id, const Dictionary &amap):
        SQLiteTableView(con, "attachments_list_view"), attachments_map_(amap) {

        string sql("CREATE TEMPORARY VIEW attachments_list_view AS SELECT id, name, type FROM attachments WHERE route = ") ;
        sql += route_id;

        con_.execute(sql) ;

        addColumn("Name", "{{name}}") ;
        addColumn("Type", "{{type}}") ;
    }

    Variant transform(const string &key, const string &value) override {
        if ( key == "type" ) {
            auto it = attachments_map_.find(value) ;
            if ( it != attachments_map_.end() ) return it->second ;
            else return string() ;
        }
        return value ;
    }
private:
    Dictionary attachments_map_ ;
};

void AttachmentController::create(const std::string &route_id)
{
    AttachmentCreateForm form(request_, routes_) ;

    if ( request_.method_ == "POST" ) {

        if ( form.validate(request_) ) {

            auto it = request_.FILE_.find("attachment-file") ;

            routes_.createAttachment(route_id, it->second.name_, form.getValue("type"), it->second.data_, upload_folder_) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("attachment-create-dialog", ctx)}});
        }
    }
    else {
        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("attachment-create-dialog", ctx)) ;
    }

}

void AttachmentController::update(const std::string &route_id)
{
    if ( request_.method_ == "POST" ) {

        string id = request_.POST_.get("id") ;

        AttachmentUpdateForm form(con_, routes_) ;

        if ( form.validate(request_) ) {

            // write data to database

            routes_.updateAttachment(id, form.getValue("name"), form.getValue("type")) ;

            // send a success message
            response_.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            Variant ctx( Variant::Object{{"form", form.data()}} ) ;

            response_.writeJSONVariant(Variant::Object{{"success", false},
                                                       {"content", engine_.render("attachment-edit-dialog", ctx)}});
        }
    }
    else {

        const Dictionary &params = request_.GET_ ;
        string id = params.get("id") ;

        AttachmentUpdateForm form(con_, routes_) ;

        if ( id.empty() ) {
            throw HttpResponseException(Response::not_found) ;
        }

        string name, type_id ;
        if ( !routes_.getAttachment(id, name, type_id) ) {
            throw HttpResponseException(Response::not_found) ;
        }

        form.init({{"name", name}, {"type", type_id}}) ;

        Variant ctx( Variant::Object{{"form", form.data()}} ) ;

        response_.write(engine_.render("attachment-edit-dialog", ctx)) ;
    }

}

void AttachmentController::remove(const std::string &route_id)
{
   const Dictionary &params = request_.POST_ ;
   string id = params.get("id") ;

    if ( id.empty() )
        throw HttpResponseException(Response::not_found) ;
    else {
        if ( routes_.removeAttachment(id) )
            response_.writeJSON("{}") ;
        else
            throw HttpResponseException(Response::not_found) ;
    }
}

bool AttachmentController::dispatch()
{
    Dictionary attributes ;

    bool logged_in = user_.check() ;

    if ( request_.matches("GET", "/attachments/list/{id:\\d+}", attributes) ) {
        if ( logged_in ) list(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/attachments/add/{id:\\d+}", attributes) ) {
        if ( logged_in ) create(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", "/attachments/update/{id:\\d+}", attributes) ) {
        if ( logged_in ) update(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", "/attachments/delete/{id:\\d+}", attributes) ) {
        if ( logged_in ) remove(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    } else return false ;
}

void AttachmentController::list(const std::string &route_id)
{
    AttachmentTableView view(con_, route_id, routes_.getAttachmentsDict()) ;
    uint offset = request_.GET_.value<int>("page", 1) ;
    uint results_per_page = request_.GET_.value<int>("total", 10) ;

    Variant data = view.fetch(offset, results_per_page) ;

    response_.write(engine_.render("attachments-table-view", data )) ;
}

