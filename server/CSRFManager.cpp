//
// Created by Jordan Bare on 7/6/18.
//

#include <random>
#include <iostream>
#include <sstream>
#include "CSRFManager.h"

std::string CSRFManager::addToken() {
    std::random_device rd;
    static thread_local std::mt19937 re{rd()};
    std::string csrfToken;
    std::uniform_int_distribution<int> urd(97,122);
    do {
        std::stringstream randomStream;
        for(int i=0;i < 20;++i){
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

void CSRFManager::removeToken(const std::string &csrfToken) {
    mSetAccessMutex.lock();
    mCSRFSet.erase(csrfToken);
    mSetAccessMutex.unlock();
}
