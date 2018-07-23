//
// Created by Jordan Bare on 7/12/18.
//

#ifndef SERVER_CREDENTIALS_H
#define SERVER_CREDENTIALS_H

#include <string>
#include <vector>

class Credentials {
public:
    Credentials(std::string userName, std::string password, std::vector<unsigned char> salt);
    ~Credentials();
    std::string &getUser();
    std::string &getPassword();
    std::vector<unsigned char>& getSalt();
private:
    std::string mUserName;
    std::string mPassword;
    std::vector<unsigned char> mSalt;
};

#endif //SERVER_CREDENTIALS_H
