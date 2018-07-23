//
// Created by Jordan Bare on 7/12/18.
//

#include <cereal/archives/portable_binary.hpp>
#include "CredentialsManager.h"
#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <random>

CredentialsManager::CredentialsManager() {
    std::cout << "Enter your name:" << std::endl;
    std::string user = "user";
    //std::cin >> user;
    std::cout << "Enter your password" << std::endl;
    std::string password = "password";
    //std::cin >> password;
    std::random_device rd;
    static thread_local std::mt19937 re{rd()};
    std::uniform_int_distribution<int> urd(0,255);
    std::vector<unsigned char> salt;
    salt.reserve(32);
    for(int i = 0; i < 32; ++i){
        salt.emplace_back(static_cast<unsigned char>(char(urd(re))));
    }
    hashCredential(password, salt);
    mServerCredentials = std::make_unique<Credentials>(user, password, salt);
    OPENSSL_cleanse(&user,user.length());
    OPENSSL_cleanse(&password,password.length());
    OPENSSL_cleanse(&salt,salt.size());
}

bool CredentialsManager::compareCredentials(std::string &body) {
    std::string usrAttribute = "usr=";
    std::string pwdAttribute = "\npwd=";
    unsigned long usrLoc = body.find(usrAttribute);
    unsigned long pwdLoc = body.find(pwdAttribute);
    unsigned long csrfLoc = body.find("\n_csrf=");
    std::string user = body.substr(usrLoc + usrAttribute.length(), (pwdLoc - (usrLoc + usrAttribute.length())) - 1);
    std::string password = body.substr(pwdLoc + pwdAttribute.length(), (csrfLoc - (pwdLoc + pwdAttribute.length())) - 1);
    OPENSSL_cleanse(&body, body.length());

    if(user == mServerCredentials->getUser()){
        std::cout << "User pass" << std::endl;
        if(compareCredential(password, mServerCredentials->getPassword(), mServerCredentials->getSalt())){
            std::cout << mServerCredentials->getPassword() << " : " << password << std::endl;
            cleanseCredentials(user,password);
            return true;
        }
        std::cout << "Password pass" << std::endl;
    }
    cleanseCredentials(user,password);
    return false;
}

bool CredentialsManager::compareCredential(std::string &sessionCredential,std::string &serverCredential,std::vector<unsigned char> &serverSalt) {
    hashCredential(sessionCredential,serverSalt);
    std::cout << sessionCredential << " : " << serverCredential << std::endl;
    return serverCredential == sessionCredential;
}

void CredentialsManager::cleanseCredentials(std::string &user, std::string &password) {
    OPENSSL_cleanse(&user, user.length());
    OPENSSL_cleanse(&password, password.length());
}

void CredentialsManager::hashCredential(std::string &credential, std::vector<unsigned char> &salt) {
    unsigned char digest[32];
    SHA256_CTX context;
    SHA256_Init(&context);
    SHA256_Update(&context, credential.data(), credential.length());
    SHA256_Update(&context, salt.data(), 32);
    SHA256_Final(digest, &context);
    credential.clear();
    credential.append(reinterpret_cast<char*>(digest), sizeof(digest));
    OPENSSL_cleanse(digest,strlen((char*)digest));
}
