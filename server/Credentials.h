//
// Created by Jordan Bare on 7/12/18.
//

#ifndef SERVER_CREDENTIALS_H
#define SERVER_CREDENTIALS_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>

class Credentials {
public:
    Credentials() = default;
    Credentials(std::string userName, std::string password);
    ~Credentials();
    bool compareUserName(std::string &sessionUserName);
    bool comparePassword(std::string &sessionPassword);
private:
    std::string hashCredentials(const std::string &credential);
    std::string mUserName;
    std::string mPassword;
    friend class cereal::access;
    template <class Archive> void serialize(Archive &ar) {
        ar(mUserName,mPassword);
    }
};

#endif //SERVER_CREDENTIALS_H
