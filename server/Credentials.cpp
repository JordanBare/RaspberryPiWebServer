//
// Created by Jordan Bare on 7/12/18.
//

#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <utility>
#include "Credentials.h"

Credentials::Credentials(std::string userName, std::string password, std::vector<unsigned char> salt): mUserName(std::move(userName)),
                                                                                            mPassword(std::move(password)),
                                                                                            mSalt(std::move(salt)) {}

Credentials::~Credentials() {
    OPENSSL_cleanse(&mUserName, mUserName.length());
    OPENSSL_cleanse(&mPassword, mPassword.length());
    OPENSSL_cleanse(&mSalt, 32);
}

std::string &Credentials::getUser() {
    return mUserName;
}

std::string &Credentials::getPassword() {
    return mPassword;
}

std::vector<unsigned char>& Credentials::getSalt() {
    return mSalt;
}