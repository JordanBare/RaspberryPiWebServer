//
// Created by Jordan Bare on 7/12/18.
//

#include <openssl/crypto.h>
#include "Credentials.h"

Credentials::Credentials(std::string userName, std::string password): mUserName(std::move(userName)), mPassword(std::move(password)) {}

Credentials::~Credentials() {
    OPENSSL_cleanse(&mUserName, mUserName.length());
    OPENSSL_cleanse(&mPassword, mPassword.length());
}

bool Credentials::compareUserName(std::string &sessionUserName) {
    if(sessionUserName == mUserName){
        OPENSSL_cleanse(&sessionUserName, sessionUserName.length());
        return true;
    }
    OPENSSL_cleanse(&sessionUserName, sessionUserName.length());
    return false;
}

bool Credentials::comparePassword(std::string &sessionPassword) {
    if(sessionPassword == mPassword){
        OPENSSL_cleanse(&sessionPassword, sessionPassword.length());
        return true;
    }
    OPENSSL_cleanse(&sessionPassword, sessionPassword.length());
    return false;
}
