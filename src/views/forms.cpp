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
#if 0
void InputField::fillData(Variant::Object &base) const
{
    FormHandlerField::fillData(base) ;
    base.insert({"type", type_}) ;
    base.insert({"widget", "input-field"}) ;
}

FileUploadField::FileUploadField(const string &name): FormHandlerField(name) {}

void FileUploadField::fillData(Variant::Object &base) const {
    FormHandlerField::fillData(base) ;
    base.insert({"type", "file"}) ;
    base.insert({"widget", "file-upload-field"}) ;
    if ( max_file_size_ ) base.insert({"max_file_size", max_file_size_}) ;
    if ( !accept_.empty() ) base.insert({"accept", accept_}) ;
}

SelectField::SelectField(const string &name, std::shared_ptr<OptionsModel> options, bool multi): FormHandlerField(name), options_(options), multiple_(multi) {
    addValidator([&](const string &val, const FormHandlerField &) {
        Dictionary options = options_->fetch() ;
        if ( multiple_ ) {
            boost::char_separator<char> sep(" ");
            boost::tokenizer<boost::char_separator<char> > tokens(val, sep);

            for( const string &s: tokens ) {
                if ( !options.contains(s) ) {
                    throw FormHandlerFieldValidationError("Supplied value: " + val + " is not in option list") ;
                }
            }
        }
        else {
            if ( !options.contains(val) ) {
                throw FormHandlerFieldValidationError("Supplied value: " + val + " is not in option list") ;
            }
        }
    }) ;

}

void SelectField::fillData(Variant::Object &base) const
{
    FormHandlerField::fillData(base) ;
    base.insert({"multiselect", multiple_}) ;
    base.insert({"widget", "select-field"}) ;
    if ( options_ ) {
        Dictionary options = options_->fetch() ;
        Variant::Array optlist ;
        for( const auto &a: options ) {
            optlist.emplace_back(Variant::Object{{"value", a.first}, {"label", a.second}, {"select", a.first == value_ }}) ;
        }
        if ( !optlist.empty() ) base.insert({"options", optlist}) ;
    }
}


CheckBoxField::CheckBoxField(const string &name, bool is_checked): FormHandlerField(name), is_checked_(is_checked) {

}

void CheckBoxField::fillData(Variant::Object &res) const {
    FormHandlerField::fillData(res) ;
    if ( is_checked_ ) res.insert({"checked", "checked"}) ;
    res.insert({"template", "checkbox"}) ;
    res.insert({"widget", "checkbox-field"}) ;
}
/*
CSRFField::CSRFField(const string &name,  Session &session): InputField(name, "hidden"), con_(con), session_(session)
{
    string selector = encodeBase64(randomBytes(12)) ;
    string token = randomBytes(24) ;
    time_t expires = std::time(nullptr) + 3600*1; // Expire in 1 hour

    sqlite::Statement stmt(con_, "INSERT INTO auth_tokens ( user_id, selector, token, expires ) VALUES ( ?, ?, ?, ? );", user_id, selector, binToHex(hashSHA256(token)), expires) ;
    stmt.exec() ;

        response_.setCookie("auth_token", selector + ":" + encodeBase64(token),  expires, "/");
    }

    // update user info
    sqlite::Statement stmt(con_, "UPDATE user_info SET last_sign_in = ? WHERE user_id = ?", std::time(nullptr), user_id) ;
    stmt.exec() ;

}

}
*/

#endif
FormHandler::FormHandler() {}

void FormHandler::addField(const FormField::Ptr &field) {
 //   field->id(field_prefix_ + field->name_ + field_suffix_) ;
 //   field->count_ = fields_.size() ;
    fields_.push_back(field) ;
    field_map_.insert({field->getName(), field}) ;
}


Variant::Object FormHandler::view() const
{

    Variant::Object form ;

    // FormHandler global errors

    if ( !errors_.empty() ) {
        Variant::Array errors ;
        for( const string &msg: errors_ )
            errors.emplace_back(msg) ;
        form.insert({"errors", errors}) ;
    }

    // FormHandler fields

    Variant::Object fields ;

    for( const auto &p: fields_ ) {
        Variant::Object field_data ;
        p->fillData(field_data) ;
        if ( is_valid_ )
            field_data.insert({"value", p->value_}) ;
        else if ( !p->initial_value_.empty() )
            field_data.insert({"value", p->initial_value_}) ;

        fields.insert({p->name_, field_data}) ;
    }

    form.insert({"fields", fields}) ;

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
        response.writeJSONVariant(Variant::Object{{"result", view() }});
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
