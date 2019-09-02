#ifndef __WSPP_UTIL_FORMS_HPP__
#define __WSPP_UTIL_FORMS_HPP__

#include <wspp/views/validators.hpp>

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/util/variant.hpp>

#include <wspp/server/session.hpp>
#include <wspp/server/request.hpp>

#include <memory>
#include <functional>

using std::string;
using wspp::server::Request;
using wspp::server::Response;

namespace wspp {
namespace twig {
class TemplateRenderer;
}
}

namespace wspp {
namespace web {
// Form helper class. The aim of the class is:
// 1) declare form in a view agnostic way
// 2) validate input data for those fields e.g passed in a POST request via validators
// 3) controller for server side form handling
using wspp::util::Variant;
using wspp::util::Dictionary;
class FormField {
public:
    // The normalizer preprocesses an input value before passing it to validators
    typedef std::function<string (const string &)> Normalizer;
    typedef std::shared_ptr<FormField> Ptr;

public:
    FormField(const string &name): name_(name) {}

    // set field value
    FormField &value(const string &val) { value_ = val; return *this; }

    // the validator checks the validity of the input string.
    // if invalid then it should throw a FormFieldValidationError exception

    // add validator
    FormField &addValidator(FormFieldValidator *val) { validators_.emplace_back(val); return *this; }
    // add custom validator as lambda
    FormField &addValidator(ValidatorCallback val) { addValidator<CallbackValidator>(val);  return *this; }
    // validator helper
    template <class T, typename ... Args>
    FormField &addValidator(Args&&...args) {
        validators_.emplace_back(std::unique_ptr<T>(new T(std::forward<Args>(args)...))); return *this;
    }

    // set custom normalizer
    FormField &setNormalizer(Normalizer val) { normalizer_ = val; return *this; }

    FormField &initial(const string &v) { initial_value_ = v; return *this; }

    FormField &alias(const string &v) { alias_ = v; return *this; }

    bool valid() const { return is_valid_; }

    void addErrorMsg(const string &msg) { errors_.emplace_back(msg); }

    std::string getValue() const { return value_; }

    std::string getAlias() const { return alias_; }

    std::string getName() const { return name_; }

protected:
    virtual void fillData(Variant::Object &) const;

    // calls all validators
    virtual bool validate(const string &value);

    friend class FormHandler;

    string name_, value_, initial_value_, alias_;

    std::vector<string> errors_;
    std::vector<std::unique_ptr<FormFieldValidator>> validators_;
    Normalizer normalizer_;
    bool is_valid_ = false;
};

#if 0
// This is used to abstract a data source which provides key value pairs to be used e.g. in selection boxes or radio boxes
// e.g. when options have to be loaded from a database or file
class OptionsModel {
public:
    typedef std::shared_ptr<OptionsModel> Ptr;

    virtual Dictionary fetch() = 0;
};

// convenience class for wrapping a dictionary
class DictionaryOptionsModel: public OptionsModel {
public:
    DictionaryOptionsModel(const Dictionary &dict): dict_(dict) {}
    virtual Dictionary fetch() override {
        return dict_;
    }

private:
    Dictionary dict_;
};

// wrapper for a lambda
class CallbackOptionsModel: public OptionsModel {
public:
    CallbackOptionsModel(std::function<Dictionary ()> cb): cb_(cb) {}

    Dictionary fetch() override { return cb_(); }
private:
    std::function<Dictionary ()> cb_;
};

class InputField: public FormField {
public:
    InputField(const string &name, const string &type): FormField(name), type_(type) {}

    void fillData(Variant::Object &) const override;

private:
    string type_;
};

class FileUploadField: public FormField {
public:
    FileUploadField(const string &name);

    FileUploadField &maxFileSize(uint64_t sz) { max_file_size_ = sz; return *this; }
    FileUploadField &accept(const std::string &a) { accept_ = a; return *this; }

    void fillData(Variant::Object &) const override;

private:
    uint64_t max_file_size_ = 0;
    std::string accept_;
};

class SelectField: public FormField {
public:
    SelectField(const string &name, std::shared_ptr<OptionsModel> options, bool multi = false);

    void fillData(Variant::Object &) const override;

private:
    bool multiple_;
    std::shared_ptr<OptionsModel> options_;
};

class CheckBoxField: public FormField {
public:
    CheckBoxField(const string &name, bool is_checked = false);

    void fillData(Variant::Object &res) const override;

private:
    bool is_checked_ = false;
};
#endif

class FormHandler {
public:
    FormHandler();

    // helper for fluent field creation
    template<typename ... Args >
    FormField &field(Args... args) {
        auto f = std::make_shared<FormField>(args...);
        addField(f);
        return *f;
    }

    // call to validate the user data against the form
    // the field values are stored in case of succesfull field validation
    // override to add additional validation e.g. requiring more than one fields (do not forget to call base class)
    virtual bool validate(const Request &vals);

    // init form with user supplied values (no validation)
    void init(const Dictionary &vals);

    // get form view to pass to template render
    Variant::Object view() const;

    // get errors object
    Variant::Object errors() const;

    // get value stored in a field
    string getValue(const string &field_name);

    // override to perform special processing after a succesfull form validation (e.g. persistance)
    virtual void onSuccess(const Request &request) {}
    // override to perform special processing when a GET request is handled (usually init form from persistance)
    virtual void onGet(const Request &request) {}

    // Use this to avoid boilerplate in form request handling
    // When a POST request is recieved and succesfully validated then onSuccess function is called
    // When a GET request is received for initial rendering of the form then the onGet handler is called to initialize
    // the form
    void handle(const Request &req, server::Response &response, twig::TemplateRenderer &engine);

protected:
    std::vector<FormField::Ptr> fields_;
    std::map<string, FormField::Ptr> field_map_;

    std::vector<string> errors_;
    bool is_valid_ = false;

    void addField(const FormField::Ptr &field);
};
} // namespace web
} // namespace wspp
#endif
