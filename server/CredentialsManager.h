//
// Created by Jordan Bare on 7/12/18.
//

#ifndef SERVER_PASSWORDMANAGER_H
#define SERVER_PASSWORDMANAGER_H

#include <string>

class CredentialsManager {
public:
    explicit CredentialsManager(const std::string &folderRoot);
    bool compareCredentials(const std::string &body);
private:
    void checkForCredentialsFile();
    const std::string mCredentialsDirectory;
};

#endif //SERVER_PASSWORDMANAGER_H
