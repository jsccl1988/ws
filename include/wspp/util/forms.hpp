#ifndef __WSPP_UTIL_FORMS_HPP__
#define __WSPP_UTIL_FORMS_HPP__

#include <string>
#include <wspp/util/dictionary.hpp>
#include <wspp/util/variant.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

using std::string ;

namespace wspp {

class FormField {
public:
    typedef boost::function<bool (const string &, FormField &)> Validator ;

    static Validator requiredArgValidator ;
public:

    FormField(const string &name): name_(name) {
        validators_.push_back(requiredArgValidator) ;
    }

    // set field to required
    FormField &required(bool is_required = true) { required_ = is_required ; return *this ;}
    // set field to disabled
    FormField &disabled(bool is_disabled = true) { disabled_ = is_disabled ; return *this ;}
    // set field value
    FormField &value(const string &val) { value_ = val ; return *this ;}
    // append classes to class attribute
    FormField &appendClass(const string &extra) { extra_classes_ = extra ; return *this ; }
    // append extra attributes to element
    FormField &extraAttributes(const Dictionary &attrs) { extra_attrs_ = attrs ; return *this ;}
    // set custom validator
    FormField &addValidator(Validator val) { validators_.push_back(val) ; return *this ;}
    // set field id
    FormField &id(const string &id) { id_ = id ; return *this ;}
    // set label
    FormField &label(const string &label) { label_ = label ; return *this ;}
    // set placeholder
    FormField &placeholder(const string &p) { place_holder_ = p ; return *this ;}
    // set initial value
    FormField &initial(const string &v) { initial_value_ = v ; return *this ;}
    // set help text
    FormField &help(const string &text) { help_text_ = text ; return *this ;}

    void addErrorMsg(const string &msg) { error_messages_.push_back(msg) ; }

protected:
    virtual void fillData(Variant::Object &) const ;
    virtual bool validate(const string &value) ;

private:
    friend class Form ;


    string label_, name_, value_, id_, place_holder_, initial_value_, help_text_ ;
    bool required_ = false, disabled_ = false ;
    string extra_classes_ ;
    Dictionary extra_attrs_ ;
    std::vector<string> error_messages_ ;
    std::vector<Validator> validators_ ;
    uint count_ = 0 ;

};

class OptionsFetcher {
public:
    virtual Dictionary fetch() = 0 ;
};

class Form {
public:

    Form(const string &field_prefix = "", const string &field_suffix = "_field") ;

    // add an input field
    FormField &input(const string &name, const string &type) ;
    // add a select field
    FormField &select(const string &name, const Dictionary &options, bool multi = false) ;

    // add a select field
    FormField &select(const string &name, boost::function<Dictionary ()> options, bool multi = false) ;

    // add a select field
    FormField &select(const string &name, boost::shared_ptr<OptionsFetcher> options, bool multi = false) ;

    // add a checkbox
    FormField &checkbox(const string &name, bool is_checked = false) ;

    bool validate(const Dictionary &vals) ;

    void addError(const string &msg) { errors_.push_back(msg) ; }

    Variant::Object data(bool bound = false) const ;

    string getValue(const string &field_name) ;

protected:

    std::map<string, boost::shared_ptr<FormField>> fields_ ;
    string field_prefix_, field_suffix_ ;
    std::vector<string> errors_ ;
    bool is_valid_ = false ;
} ;







}

#endif
