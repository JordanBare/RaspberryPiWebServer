//
// Created by Jordan Bare on 7/12/18.
//

#ifndef SERVER_PASSWORDMANAGER_H
#define SERVER_PASSWORDMANAGER_H

#include <string>
#include "Credentials.h"

class CredentialsManager {
public:
    explicit CredentialsManager();
    bool compareCredentials(std::string &body);
private:
    void cleanseCredentials(std::string &user, std::string &password);
    bool comparePasswords(std::string &sessionPassword);
    void hashCredential(std::string &credential, std::vector<unsigned char> &salt);
    std::unique_ptr<Credentials> mServerCredentials;
};

#endif //SERVER_PASSWORDMANAGER_H
