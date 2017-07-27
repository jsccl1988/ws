#include <wspp/auth/password_authenticator.hpp>

using namespace std ;

namespace wspp {

/*
 *
 CREATE TABLE `attempts` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `ip` TEXT NOT NULL,
  `expiredate` INTEGER NOT NULL
) ;

CREATE TABLE `requests` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `uid` INTEGER NOT NULL,
  `rkey` TEXT NOT NULL,
  `expire` INTEGER NOT NULL,
  `type` TEXT NOT NULL
) ;


CREATE TABLE `tokens` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `uid` INTEGER NOT NULL,
  `hash` TEXT NOT NULL,
  `expiredate` INTEGER NOT NULL,
  `ip` TEXT NOT NULL,
  `agent` TEXT NOT NULL,
  `cookie_crc` TEXT NOT NULL
);


DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `email` TEXT DEFAULT NULL,
  `password` TEXT DEFAULT NULL,
  `active` INTEGER NOT NULL DEFAULT 0,
);
*/

void PasswordAuthenticator::login(const string &email, const string &password, bool remember) {
    // data validation

    string vusername = validateEmail(email) ;

    string vpassword = validatePassword(password) ;


}

string PasswordAuthenticator::validateEmail(const string &email)
{

}

}
