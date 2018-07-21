//
// Created by Jordan Bare on 7/6/18.
//

#include <random>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include "CSRFTokenManager.h"

CSRFTokenManager::CSRFTokenManager(sqlite3 *&database):mDatabase(database) {}

void CSRFTokenManager::printDatabaseError() {
    std::cout << sqlite3_errmsg(mDatabase) << std::endl;
}

std::string CSRFTokenManager::generateToken() {
    std::random_device rd;
    static thread_local std::mt19937 re{rd()};
    std::uniform_int_distribution<int> urd(97,122);
    std::string csrfToken;
    bool duplicate = true;
    while(duplicate){
        std::stringstream randomStream;
        for(int i = 0; i < 20; ++i){
            randomStream << char(urd(re));
        }
        csrfToken = randomStream.str();

        sqlite3_stmt *stmt;
        if(sqlite3_prepare_v2(mDatabase, "INSERT INTO csrftokens (token) VALUES (?);", -1, &stmt, nullptr) != SQLITE_OK){
            printDatabaseError();
        }
        if(sqlite3_bind_text(stmt, 1, csrfToken.c_str(), static_cast<int>(csrfToken.length()), SQLITE_TRANSIENT) != SQLITE_OK){
            printDatabaseError();
        }
        if(sqlite3_step(stmt) == SQLITE_DONE){
            duplicate = false;
        } else {
            printDatabaseError();
        }
        sqlite3_finalize(stmt);
    }
    return csrfToken;
}

void CSRFTokenManager::removeToken(const std::string &sessionToken) {
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(mDatabase, "DELETE FROM csrftokens WHERE token = ?;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_bind_text(stmt, 1, sessionToken.c_str(), static_cast<int>(sessionToken.length()), SQLITE_TRANSIENT) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_step(stmt) != SQLITE_DONE){
        printDatabaseError();
    }
    sqlite3_finalize(stmt);
}

bool CSRFTokenManager::compareSessionToken(const std::string &sessionToken, const std::string &requestBody) {
    std::string tokenAttribute = "\n_csrf=";
    unsigned long tokenIndex = requestBody.find(tokenAttribute);
    //always change indexes based on csrf token name
    std::string requestCSRFToken = requestBody.substr(tokenIndex + tokenAttribute.length(), 20);
    if(requestCSRFToken == sessionToken){
        removeToken(sessionToken);
        return true;
    }
    return false;
}

void CSRFTokenManager::insertToken(std::string &sessionToken, std::string &page) {
    sessionToken = generateToken();
    boost::replace_all(page, "CSRF", sessionToken);
}
