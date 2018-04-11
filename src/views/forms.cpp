#include <wspp/views/forms.hpp>
#include <wspp/twig/renderer.hpp>
#include <wspp/util/crypto.hpp>

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

using namespace std ;
using namespace wspp::server ;
using namespace wspp::util ;
using namespace wspp::twig ;

namespace wspp { namespace web {

void FormField::fillData(Variant::Object &res) const {

    if ( !name_.empty() ) res.insert({"name", name_}) ;

    if ( !value_.empty() ) res.insert({"value", value_}) ;
    else if ( !initial_value_.empty() ) res.insert({"value", initial_value_}) ;

    if ( !errors_.empty() ) res.insert({"errors", Variant::fromVector(errors_)}) ;
}

bool FormField::validate(const string &value)
{
    // normalize passed value

    string n_value = normalizer_ ? normalizer_(value) : value ;

    // call validators

    for( auto &v: validators_ ) {
        try {
            v->validate(n_value, *this) ;
        }
        catch ( FormFieldValidationError &e ) {
            addErrorMsg(e.what());
            return false ;
        }
    }

    // store value

    value_ = n_value ;
    is_valid_ = true ;
    return true ;
}

FormHandler::FormHandler() {}

void FormHandler::addField(const FormField::Ptr &field) {
    fields_.push_back(field) ;
    field_map_.insert({field->getName(), field}) ;
}


Variant::Object FormHandler::view() const
{
    Variant::Object form ;

    for( const auto &p: fields_ ) {
      if ( !p->value_.empty() )
            form.insert({p->name_, p->value_}) ;
    }

    return form ;
}

Variant::Object FormHandler::errors() const
{
    Variant::Object e, fields ;

    e.insert({"global_errors", Variant::fromVector(errors_)}) ;
    for( const auto &f: fields_ ) {
        fields.insert({f->name_, Variant::fromVector(f->errors_)}) ;
    }
    e.insert({"field_errors", fields}) ;

    return e ;
}

string FormHandler::getValue(const string &field_name)
{
    const auto &it = field_map_.find(field_name) ;
    assert ( it != field_map_.end() ) ;
    return it->second->value_ ;
}


void FormHandler::handle(const Request &request, Response &response, TemplateRenderer &engine)
{
    if ( request.method_ == "POST" ) {

        if ( validate(request) ) {
            onSuccess(request) ;

            // send a success message
            response.writeJSONVariant(Variant::Object{{"success", true}}) ;
        }
        else {
            response.writeJSONVariant(Variant::Object{{"success", false}, {"errors", errors() }});
        }
    }
    else {
        onGet(request) ;
        response.writeJSONVariant(view());
    }
}

bool FormHandler::validate(const Request &req) {
    bool failed = false ;

    // validate POST params

    for( const auto &p: fields_ ) {
        if ( req.POST_.contains(p->name_) ) {

            bool res = p->validate(req.POST_.get(p->name_)) ;
            if ( !res ) failed = true ;
        }
    }

    // validate file params
    // in this case the validator only receives the name of the field in the FILE dictionary
    // and is responsible for fetchinf the data from there

    for( const auto &p: fields_ ) {
        auto it = req.FILE_.find(p->name_) ;
        if ( it != req.FILE_.end() ) {
            bool res = p->validate(p->name_) ;
            if ( !res ) failed = true ;
        }
    }

    is_valid_ = !failed ;
    return is_valid_ ;
}

void FormHandler::init(const Dictionary &vals) {
    for( const auto &p: vals ) {
        auto it = field_map_.find(p.first) ;
        if ( it != field_map_.end() ) {
            it->second->value(p.second) ;
        }
    }
}


} // namespace web
} // namespace wspp
