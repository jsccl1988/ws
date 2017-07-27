#ifndef __WSPP_PASSWORD_AUTHENTICATOR_HPP__
#define __WSPP_PASSWORD_AUTHENTICATOR_HPP__

#include <wspp/util/database.hpp>
#include <boost/make_shared.hpp>
namespace wspp {

class PasswordValidator {
public:
    virtual std::string validate(const std::string &password) = 0 ;
    using Ptr = boost::shared_ptr<PasswordValidator> ;
};

class DefaultPasswordValidator: public PasswordValidator {
public:
    std::string validate(const std::string &password) override ;
};


class PasswordAuthenticator {
public:
    PasswordAuthenticator(sqlite::Connection &con, PasswordValidator::Ptr pv = boost::make_shared<DefaultPasswordValidator>()): db_(con), password_validator_(pv) {}

    void login(const std::string &email, const std::string &password, bool remember = false) ;

protected:

    std::string validateEmail(const std::string &email) ;
    std::string validatePassword(const std::string &password) {
        return password_validator_->validate(password) ;
    }

protected:

    PasswordValidator::Ptr password_validator_ ;
    sqlite::Connection &db_ ;
};
}








#endif
