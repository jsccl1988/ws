#ifndef __WSPP_UTIL_FORMS_HPP__
#define __WSPP_UTIL_FORMS_HPP__

#include <wspp/views/validators.hpp>

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/util/variant.hpp>
#include <wspp/util/database.hpp>
#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>

#include <memory>
#include <functional>

using std::string ;
using wspp::server::Request ;

namespace wspp { namespace web {

// Form helper class. The aim of the class is:
// 1) declare form in a view agnostic way
// 2) validate input data for those fields e.g passed in a POST request via validators
// 3) pack data needed to render the form into an object that can be passed to the template engine

using wspp::util::Variant ;
using wspp::util::Dictionary ;

class FormField: public boost::noncopyable {
public:

    // The normalizer preprocesses an input value before passing it to validators
    typedef std::function<string (const string &)> Normalizer ;

    typedef std::shared_ptr<FormField> Ptr ;

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

    // the validator checks the validity of the input string.
    // if invalid then it should throw a FormFieldValidationError exception

    // add validator
    FormField &addValidator(FormFieldValidator *val) { validators_.emplace_back(val) ; return *this ; }
    // add custom validator as lambda
    FormField &addValidator(ValidatorCallback val) { addValidator<CallbackValidator>(val) ;  return *this ; }
    // validator helper
    template <class T, typename ... Args>
    FormField &addValidator(Args&&...args) {
        validators_.emplace_back(std::unique_ptr<T>(new T(std::forward<Args>(args)...))); return *this ;
    }

    // set custom normalizer
    FormField &setNormalizer(Normalizer val) { normalizer_ = val ; return *this ; }
    // set field id
    FormField &id(const string &id) { id_ = id ; return *this ; }
    // set label
    FormField &label(const string &label) { label_ = label ; return *this ; }

    // set alias
    FormField &alias(const string &alias) { alias_ = alias ; return *this ; }

    // set placeholder
    FormField &placeholder(const string &p) { place_holder_ = p ; return *this ; }
    // set initial value
    FormField &initial(const string &v) { initial_value_ = v ; return *this ; }
    // set help text
    FormField &help(const string &text) { help_text_ = text ; return *this ; }

    bool valid() const { return is_valid_ ; }
    void addErrorMsg(const string &msg) { error_messages_.push_back(msg) ; }
    std::string getValue() const { return value_ ; }

    // returns a name of the field to be used in validation errors
    // if not alias has been defined then the label uncapitilized is used and if this is not defined then the name is used.
    std::string getAlias() const ;


protected:
    virtual void fillData(Variant::Object &) const ;
    // calls all validators
    virtual bool validate(const string &value) ;

    friend class Form ;

    string label_, name_, alias_, value_, id_, place_holder_, initial_value_, help_text_ ;
    bool required_ = false, disabled_ = false ;
    string extra_classes_ ;
    Dictionary extra_attrs_ ;
    std::vector<string> error_messages_ ;
    std::vector<std::unique_ptr<FormFieldValidator>> validators_ ;
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
    CallbackOptionsModel(std::function<Dictionary ()> cb): cb_(cb) {}

    Dictionary fetch() override { return cb_() ; }
private:
    std::function<Dictionary ()> cb_ ;
};


class InputField: public FormField {
public:
    InputField(const string &name, const string &type): FormField(name), type_(type) {}

    void fillData(Variant::Object &) const override;
private:
    string type_ ;
};

class FileUploadField: public FormField {
public:
    FileUploadField(const string &name);

    FileUploadField &maxFileSize(uint64_t sz) { max_file_size_ = sz ; return *this ; }
    FileUploadField &accept(const std::string &a) { accept_ = a ; return *this ; }

    void fillData(Variant::Object &) const override;
private:
    uint64_t max_file_size_ = 0 ;
    std::string accept_ ;
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


class Form {
public:

    Form(const string &field_prefix = "", const string &field_suffix = "_field") ;

    template<typename T, typename ... Args >
    T &field(Args... args) {
        auto f = std::make_shared<T>(args...) ;
        addField(f) ;
        return *f ;
    }

    // call to validate the user data against the form
    // the field values are stored in case of succesfull field validation
    // override to add additional validation e.g. requiring more than one fields (do not forget to call base class)
    virtual bool validate(const Request &vals) ;

    // init values with user supplied (no validation)
    void init(const Dictionary &vals) ;

    // get form view to pass to template
    Variant::Object data() const ;

    // get errors object
    Variant::Object errors() const ;

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
