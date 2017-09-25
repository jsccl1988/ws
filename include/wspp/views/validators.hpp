#ifndef __FORM_FIELD_VALIDATORS_HPP__
#define __FORM_FILED_VALIDATORS_HPP__

#include <wspp/util/dictionary.hpp>
#include <boost/regex.hpp>

#include <stdexcept>
#include <functional>

using wspp::util::Dictionary ;

namespace wspp {
namespace web {

// exception thrown by validators
class FormFieldValidationError: public std::runtime_error {
public:

    FormFieldValidationError(const std::string &msg): std::runtime_error(msg) {}
};

class FormField ;

class FormFieldValidator {
public:
    virtual void validate(const std::string &val, const FormField &field) const = 0 ;

    // the function is used to interpolate a template string with variables contained in params and implicitly declared variables
    // such as {field} and {value} obtained from the field and passed value respectively
    static std::string interpolateMessage(const std::string &msg_template, const std::string &value, const FormField &field, const Dictionary &params= Dictionary()) ;

    virtual ~FormFieldValidator() {}
};

typedef std::function<void (const std::string &, const FormField &)> ValidatorCallback ;

class CallbackValidator: public FormFieldValidator {
public:

    CallbackValidator(ValidatorCallback v): cb_(v) {}

    virtual void validate(const std::string &val, const FormField &field) const {
        cb_(val, field) ;
    }
private:
    ValidatorCallback cb_ ;
};

class NonEmptyValidator: public FormFieldValidator {
public:
    NonEmptyValidator(const std::string &msg = std::string()):  msg_(msg) {}

    virtual void validate(const std::string &val, const FormField &field) const override ;
protected:
    std::string msg_ ;
private:
    const static std::string validation_msg_ ;
};

class MinLengthValidator: public FormFieldValidator {
public:
    MinLengthValidator(uint min_length, const std::string &msg = std::string()): min_len_(min_length), msg_(msg) {}

    virtual void validate(const std::string &val, const FormField &field) const override ;
protected:
    std::string msg_ ;
    uint min_len_ ;
private:
    const static std::string validation_msg_ ;
};

class MaxLengthValidator: public FormFieldValidator {
public:
    MaxLengthValidator(uint max_length, const std::string &msg = std::string()): max_len_(max_length), msg_(msg) {}

    virtual void validate(const std::string &val, const FormField &field) const override ;
protected:
    std::string msg_ ;
    uint max_len_ ;
private:
    const static std::string validation_msg_ ;
};

class RegexValidator: public FormFieldValidator {
public:
    RegexValidator(const boost::regex &rx, const std::string &msg, bool negative = false): rx_(rx), msg_(msg), is_negative_(false) {}

    virtual void validate(const std::string &val, const FormField &field) const override ;
protected:
    boost::regex rx_ ;
    std::string msg_ ;
    bool is_negative_ ;
private:
    const static std::string validation_msg_ ;
};









}
}

#endif