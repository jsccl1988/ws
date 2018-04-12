#include "attachment_controller.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include <wspp/views/table.hpp>
#include <wspp/server/exceptions.hpp>

using namespace std ;
using namespace wspp::util ;
using namespace wspp::web ;
using namespace wspp::server ;

static Route route_list("/attachments/list/{id:\\d+}") ;
static Route route_add("/attachments/add/{id:\\d+}") ;
static Route route_update("/attachments/update/{id:\\d+}") ;
static Route route_delete("/attachments/delete/{id:\\d+}") ;

AttachmentCreateForm::AttachmentCreateForm(const Request &req, RouteModel &routes, const string &route_id,
                                           const string &upload_folder):
    request_(req), routes_(routes), upload_folder_(upload_folder), route_id_(route_id) {

    field("type").alias("Type") ;

    /*, std::make_shared<DictionaryOptionsModel>(routes_.getAttachmentsDict()))
    .required().label("Type") ;
*/

    const size_t max_attachment_file_size = 5 * 1024 * 1024;

    field("attachment-file").alias("Attachment") //.maxFileSize(max_attachment_file_size).label("Attachment").required()
        .addValidator([&] (const string &val, const FormField &f) {
            auto it = request_.FILE_.find("attachment-file") ;
            if ( it == request_.FILE_.end() )
                throw FormFieldValidationError("No file received") ;
    }) ;
}

void AttachmentCreateForm::onSuccess(const Request &request)
{
   auto it = request.FILE_.find("attachment-file") ;

   routes_.createAttachment(route_id_, it->second.name_, getValue("type"), it->second.data_, upload_folder_) ;
}


AttachmentUpdateForm::AttachmentUpdateForm(RouteModel &routes, const string &route_id):
    route_id_(route_id), routes_(routes) {

    field("name").alias("Name")
        .addValidator<NonEmptyValidator>() ;

    field("type").alias("Type") ;//, std::make_shared<DictionaryOptionsModel>(routes_.getAttachmentsDict()))
}

void AttachmentUpdateForm::onSuccess(const Request &request) {
    string id = request.POST_.get("id") ;

    // write data to database

    routes_.updateAttachment(id, getValue("name"), getValue("type")) ;
}

void AttachmentUpdateForm::onGet(const Request &request)
{
    const Dictionary &params = request.GET_ ;
    string id = params.get("id") ;

    if ( id.empty() ) {
        throw HttpResponseException(Response::not_found) ;
    }

    string name, type_id ;
    if ( !routes_.getAttachment(id, name, type_id) ) {
        throw HttpResponseException(Response::not_found) ;
    }

    init({{"name", name}, {"type", type_id}}) ;
}


class AttachmentTableView: public SQLTableView {
public:
    AttachmentTableView(Connection &con, const std::string &route_id, const Dictionary &amap):
        SQLTableView(con, "attachments_list_view"), attachments_map_(amap) {


        string sql("CREATE TEMPORARY VIEW attachments_list_view AS SELECT id, name, type FROM attachments WHERE route = ") ;
        sql += route_id;

        con_.execute(sql) ;
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
    AttachmentCreateForm form(request_, routes_, route_id, upload_folder_) ;

    form.handle(request_, response_, engine_) ;
}

void AttachmentController::update(const std::string &route_id)
{
    AttachmentUpdateForm form(routes_, route_id) ;

    form.handle(request_, response_, engine_) ;
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

    if ( request_.matches("GET", route_list, attributes) ) {
        if ( logged_in ) list(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", route_add, attributes) ) {
        if ( logged_in ) create(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    if ( request_.matches("GET|POST", route_update, attributes) ) {
        if ( logged_in ) update(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    }
    else if ( request_.matches("POST", route_delete, attributes) ) {
        if ( logged_in ) remove(attributes.get("id")) ;
        else throw HttpResponseException(Response::unauthorized) ;
        return true ;
    } else return false ;
}

void AttachmentController::list(const std::string &route_id)
{
    AttachmentTableView view(con_, route_id, routes_.getAttachmentsDict()) ;
    view.handle(request_, response_) ;
}

