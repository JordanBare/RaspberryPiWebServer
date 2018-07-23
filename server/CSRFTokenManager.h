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
    /*
    std::uniform_int_distribution<int> mRandURD = (1,3);
    std::uniform_int_distribution<int> mNumURD(48,57);
    std::uniform_int_distribution<int> mCapURD(65,90);
    std::uniform_int_distribution<int> mLowURD(97,122);
     */
};

#endif //SERVER_CSRFMANAGER_H
