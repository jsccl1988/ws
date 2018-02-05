#include <wspp/views/forms.hpp>

namespace wspp {
namespace web {


std::string FormFieldValidator::interpolateMessage(const std::string &msg_template, const std::string &value, const FormField &field, const util::Dictionary &params) {

    string res, param ;
    bool in_param = false ;
    string::const_iterator cursor = msg_template.begin(), end = msg_template.end() ;

    while ( cursor != end ) {
        char c = *cursor++ ;
        if ( c == '{' ) {
            in_param = true ;
            param.clear() ;
        }
        else if ( c == '}' ) {
            in_param = false ;

            if ( param.empty() ) ;
            else if ( param == "field" ) {
                res.append(field.getAlias()) ;
            } else if ( param == "value" ) {
                res.append(value) ;
            }
            else {
                string p = params.get(param) ;
                if ( !p.empty() )
                    res.append(p) ;
            }
        }
        else if ( c == '\\' && !in_param) {
            char c = *cursor ;
            switch (c) {
                case '{':
                case '}':
                case '\\':
                    res.push_back(c) ;
                    break ;
                default:
                    res.push_back('\\') ;
                    res.push_back(c) ;
            }
        }
        else if ( in_param ) {
            param.push_back(c) ;
        }
        else res.push_back(c) ;
    }

    return res ;
}



const std::string NonEmptyValidator::validation_msg_ = "{field} cannot be empty";

void NonEmptyValidator::validate(const std::string &val, const FormField &field) const {
    if ( val.empty() ) {
        throw FormFieldValidationError( FormFieldValidator::interpolateMessage(msg_.empty() ? validation_msg_ : msg_, val, field )) ;
    }
}

const std::string MinLengthValidator::validation_msg_ = "{field} should contain at least {min} characters";

void MinLengthValidator::validate(const std::string &val, const FormField &field) const
{
    size_t len = val.length() ;

    if ( len < min_len_ ) {
        string cmsg = std::to_string(len)  ;
        throw FormFieldValidationError( FormFieldValidator::interpolateMessage(msg_.empty() ? validation_msg_ : msg_, val, field, {{"min",  cmsg}} )) ;
    }

}

const std::string MaxLengthValidator::validation_msg_ = "{field} should contain less than {max} characters";

void MaxLengthValidator::validate(const std::string &val, const FormField &field) const
{
    size_t len = val.length() ;

    if ( len > max_len_ ) {
        string cmsg = std::to_string(len)  ;
        throw FormFieldValidationError( FormFieldValidator::interpolateMessage(msg_.empty() ? validation_msg_ : msg_, val, field, {{"max",  cmsg}} )) ;
    }
}

void RegexValidator::validate(const std::string &val, const FormField &field) const
{
    if ( !is_negative_ && !boost::regex_match(val, rx_) ) {
        throw FormFieldValidationError( FormFieldValidator::interpolateMessage(msg_, val, field) ) ;
    }
}


}
}
