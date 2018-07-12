//
// Created by Jordan Bare on 7/6/18.
//

#ifndef SERVER_CSRFMANAGER_H
#define SERVER_CSRFMANAGER_H

#include <mutex>
#include <set>

class CSRFManager {
public:
    void insertToken(std::string &sessionToken, std::string &page);
    void removeToken(const std::string &sessionToken);
    bool compareSessionToken(const std::string &sessionToken, const std::string &requestBody);
private:
    std::string generateToken();
    bool checkSetForToken(const std::string &csrfToken);
    std::set<std::string> mCSRFSet;
    std::mutex mSetAccessMutex;
};

#endif //SERVER_CSRFMANAGER_H
