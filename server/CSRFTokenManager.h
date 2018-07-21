//
// Created by Jordan Bare on 7/6/18.
//

#ifndef SERVER_CSRFMANAGER_H
#define SERVER_CSRFMANAGER_H

#include <sqlite3.h>

class CSRFTokenManager {
public:
    explicit CSRFTokenManager(sqlite3 *&database);
    void insertToken(std::string &sessionToken, std::string &page);
    void removeToken(const std::string &sessionToken);
    bool compareSessionToken(const std::string &sessionToken, const std::string &requestBody);
private:
    void printDatabaseError();
    std::string generateToken();

    sqlite3 *&mDatabase;
};

#endif //SERVER_CSRFMANAGER_H
