//
// Created by Jordan Bare on 7/6/18.
//

#ifndef SERVER_CSRFMANAGER_H
#define SERVER_CSRFMANAGER_H

#include <mutex>
#include <set>

class CSRFManager {
public:
    std::string addToken();
    void removeToken(const std::string &csrfToken);
private:
    bool checkSetForToken(const std::string &csrfToken);
    std::set<std::string> mCSRFSet;
    std::mutex mSetAccessMutex;
};


#endif //SERVER_CSRFMANAGER_H
