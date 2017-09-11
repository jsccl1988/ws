#ifndef __WSPP_UTIL_FORMS_HPP__
#define __WSPP_UTIL_FORMS_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/util/variant.hpp>
#include <wspp/util/database.hpp>
#include <wspp/server/session.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

using std::string ;
using wspp::util::sqlite::Connection ;

namespace wspp { namespace web {

// Form helper class. The aim of the class is:
// 1) declare form in a view agnostic way
// 2) validate input data for those fields e.g passed in a POST request
// 3) pack data needed to render the form into an object that can be passed to the template engine

using wspp::util::Variant ;
using wspp::util::Dictionary ;

class FormFieldValidationError: public std::runtime_error {
public:
    FormFieldValidationError(const std::string &msg): std::runtime_error(msg) {}
};

class FormField {
public:
    // the validator checks the validity of the input string.
    // if invalid then it should throw a FormFieldValidationError exception

    typedef boost::function<void (const string &, FormField &)> Validator ;

    // The normalizer preprocesses an input value before passing it to validators
    typedef boost::function<string (const string &)> Normalizer ;

    typedef boost::shared_ptr<FormField> Ptr ;

public:

    FormField(const string &name): name_(name) {

    }

    // set field to required
    FormField &required(bool is_required = true) { required_ = is_required ; return *this ; }
    // set field to disabled
    FormField &disabled(bool is_disabled = true) { disabled_ = is_disabled ; return *this ; }
    // set field value
    FormField &value(const string &val) { value_ = val ; return *this ; }
    // append classes to class attribute
    FormField &appendClass(const string &extra) { extra_classes_ = extra ;  return *this ; }
    // append extra attributes to element
    FormField &extraAttributes(const Dictionary &attrs) { extra_attrs_ = attrs ; return *this ; }
    // set custom validator
    FormField &addValidator(Validator val) { validators_.push_back(val) ; return *this ; }
    // set custom normalizer
    FormField &setNormalizer(Normalizer val) { normalizer_ = val ; return *this ; }
    // set field id
    FormField &id(const string &id) { id_ = id ; return *this ; }
    // set label
    FormField &label(const string &label) { label_ = label ; return *this ; }
    // set placeholder
    FormField &placeholder(const string &p) { place_holder_ = p ; return *this ; }
    // set initial value
    FormField &initial(const string &v) { initial_value_ = v ; return *this ; }
    // set help text
    FormField &help(const string &text) { help_text_ = text ; return *this ; }

    bool valid() const { return is_valid_ ; }
    void addErrorMsg(const string &msg) { error_messages_.push_back(msg) ; }
    std::string getValue() const { return value_ ; }

public:

    // stock validators

    void validateNonEmptyField(const string &value, const std::string &field_print_Name = "Field") ;
    void validateLength(const string &value, uint min_chars, uint max_chars, const string &field_print_Name = "Field") ;

protected:
    virtual void fillData(Variant::Object &) const ;
    // calls all validators
    virtual bool validate(const string &value) ;

private:
    friend class Form ;

    string label_, name_, value_, id_, place_holder_, initial_value_, help_text_ ;
    bool required_ = false, disabled_ = false ;
    string extra_classes_ ;
    Dictionary extra_attrs_ ;
    std::vector<string> error_messages_ ;
    std::vector<Validator> validators_ ;
    Normalizer normalizer_ ;
    uint count_ = 0 ;
    bool is_valid_ = false ;

};

// This is used to abstract a data source which provides key value pairs to be used e.g. in selection boxes or radio boxes
// e.g. when options have to be loaded from a database or file

class OptionsModel {
public:
    typedef boost::shared_ptr<OptionsModel> Ptr ;

    virtual Dictionary fetch() = 0 ;
};

// convenience class for wrapping a dictionary

class DictionaryOptionsModel: public OptionsModel {
public:
    DictionaryOptionsModel(const Dictionary &dict): dict_(dict) {}
    virtual Dictionary fetch() override {
        return dict_ ;
    }

private:
    Dictionary dict_ ;
};

// wrapper for a lambda
class CallbackOptionsModel: public OptionsModel {
public:
    CallbackOptionsModel(boost::function<Dictionary ()> cb): cb_(cb) {}

    Dictionary fetch() override { return cb_() ; }
private:
    boost::function<Dictionary ()> cb_ ;
};


class InputField: public FormField {
public:
    InputField(const string &name, const string &type): FormField(name), type_(type) {}

    void fillData(Variant::Object &) const override;
private:
    string type_ ;
};

class SelectField: public FormField {
public:
    SelectField(const string &name, boost::shared_ptr<OptionsModel> options, bool multi = false);

    void fillData(Variant::Object &) const override;

private:
    bool multiple_ ;
    boost::shared_ptr<OptionsModel> options_ ;
};

class CheckBoxField: public FormField {
public:
    CheckBoxField(const string &name, bool is_checked = false);

    void fillData(Variant::Object &res) const override;
private:
    bool is_checked_ = false ;
};

class CSRFField: public InputField {
public:
    CSRFField(const std::string &name, Connection &con, wspp::server::Session &session);

    void fillData(Variant::Object &) const override;

private:

    wspp::server::Session &session_ ;
    Connection &con_ ;
};

class Form {
public:

    Form(const string &field_prefix = "", const string &field_suffix = "_field") ;

    template<typename T, typename ... Args >
    T &field(Args... args) {
        auto f = boost::make_shared<T>(args...) ;
        addField(f) ;
        return *f ;
    }

    // call to validate the user data against the form
    // the field values are stored in case of succesfull field validation
    // override to add additional validation e.g. requiring more than one fields (do not forget to call base class)
    virtual bool validate(const Dictionary &vals) ;

    // init values with user supplied (no validation)
    void init(const Dictionary &vals) ;

    Variant::Object data() const ;

    string getValue(const string &field_name) ;

protected:

    std::vector<FormField::Ptr> fields_ ;
    std::map<string, FormField::Ptr> field_map_ ;
    string field_prefix_, field_suffix_ ;
    std::vector<string> errors_ ;
    bool is_valid_ = false ;

    void addField(const FormField::Ptr &field);
} ;


} // namespace web
} // namespace wspp

#endif
