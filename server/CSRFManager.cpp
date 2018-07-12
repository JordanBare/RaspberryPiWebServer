//
// Created by Jordan Bare on 7/6/18.
//

#include <random>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include "CSRFManager.h"

std::string CSRFManager::generateToken() {
    std::random_device rd;
    static thread_local std::mt19937 re{rd()};
    std::string csrfToken;
    std::uniform_int_distribution<int> urd(97,122);
    do {
        std::stringstream randomStream;
        for(int i = 0; i < 20; ++i){
            randomStream << char(urd(re));
        }
        csrfToken = randomStream.str();
    } while(checkSetForToken(csrfToken));
    mSetAccessMutex.lock();
    mCSRFSet.emplace(csrfToken);
    mSetAccessMutex.unlock();
    return csrfToken;
}

bool CSRFManager::checkSetForToken(const std::string &csrfToken) {
    mSetAccessMutex.lock();
    bool found = mCSRFSet.find(csrfToken) != mCSRFSet.end();
    mSetAccessMutex.unlock();
    return found;
}

void CSRFManager::removeToken(const std::string &sessionToken) {
    mSetAccessMutex.lock();
    mCSRFSet.erase(sessionToken);
    mSetAccessMutex.unlock();
}

bool CSRFManager::compareSessionToken(const std::string &sessionToken, const std::string &requestBody) {
    unsigned long tokenIndex = requestBody.find("_csrf=");
    //always change indexes based on csrf token name
    std::string requestCSRFToken = requestBody.substr(tokenIndex + 6, 20);
    if(requestCSRFToken == sessionToken){
        removeToken(sessionToken);
        return true;
    }
    return false;
}

void CSRFManager::insertToken(std::string &sessionToken, std::string &page) {
    sessionToken = generateToken();
    boost::replace_all(page, "CSRF", sessionToken);
}

