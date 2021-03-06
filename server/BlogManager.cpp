//
// Created by Jordan Bare on 7/6/18.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include "BlogManager.h"
#include "Blog.h"

BlogManager::BlogManager(sqlite3 *&database): mDatabase(database), mGetBlogIdRegexFormula("^/[1-9][0-9]{0,4}") {
}

void BlogManager::printDatabaseError() {
    std::cout << sqlite3_errmsg(mDatabase) << std::endl;
}

void BlogManager::createBlogFromSubmission(const std::string &blogContent) {
    std::string titleAttribute = "title=";
    std::string blogAttribute = "\nblog=";
    std::string csrfAttribute = "\n_csrf=";
    unsigned long titleLoc = blogContent.find(titleAttribute);
    unsigned long blogLoc = blogContent.find(blogAttribute);
    unsigned long tokenLoc = blogContent.find(csrfAttribute);
    std::string title = blogContent.substr(titleLoc + titleAttribute.length(), (blogLoc - blogAttribute.length()) - 1);
    std::string content = blogContent.substr(blogLoc + blogAttribute.length(), (tokenLoc - (blogLoc + blogAttribute.length())) - 1);

    std::stringstream stream;
    stream << boost::posix_time::second_clock::local_time();
    const std::string dateTime = stream.str();

    if(!checkForBlogByTitle(title)){
        sqlite3_stmt *stmt;
        if(sqlite3_prepare_v2(mDatabase, "INSERT INTO blogs (title, datetime, content) VALUES (?, ?, ?);", -1, &stmt, nullptr) != SQLITE_OK){
            printDatabaseError();
        }
        sqlite3_bind_text(stmt, 1, title.c_str(), static_cast<int>(title.length()), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, dateTime.c_str(), static_cast<int>(dateTime.length()), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, content.c_str(), static_cast<int>(content.length()), SQLITE_TRANSIENT);
        if(sqlite3_step(stmt) != SQLITE_DONE){
            printDatabaseError();
        }
        sqlite3_finalize(stmt);
        updateBlogIndex();
    }
}

int BlogManager::getFormattedLinkToPreviousBlog(const int &id) {
    sqlite3_stmt *stmt;
    int prevId = 0;
    if(sqlite3_prepare_v2(mDatabase, "SELECT id FROM blogs WHERE id < ? ORDER BY id DESC LIMIT 1;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_bind_int(stmt, 1, id) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        prevId = sqlite3_column_int(stmt, 0);
    } else {
        printDatabaseError();
    }
    sqlite3_finalize(stmt);

    return prevId;
}

int BlogManager::getFormattedLinkToNextBlog(const int &id) {
    sqlite3_stmt *stmt;
    int nextId = 0;
    if(sqlite3_prepare_v2(mDatabase, "SELECT id FROM blogs WHERE id > ? ORDER BY id LIMIT 1;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_bind_int(stmt, 1, id) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        nextId = sqlite3_column_int(stmt, 0);
    } else {
        printDatabaseError();
    }
    sqlite3_finalize(stmt);

    return nextId;
}

std::string BlogManager::retrieveMostRecentBlog() {
    std::stringstream blogStream;
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(mDatabase, "SELECT * FROM blogs ORDER BY id DESC LIMIT 1;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        int blogId = sqlite3_column_int(stmt,0);
        std::unique_ptr blog = std::make_unique<Blog>(
                reinterpret_cast<char const *>(sqlite3_column_text(stmt, 1)),
                reinterpret_cast<char const *>(sqlite3_column_text(stmt, 2)),
                reinterpret_cast<char const *>(sqlite3_column_text(stmt, 3)),
                getFormattedLinkToPreviousBlog(blogId),
                0);
        {
            cereal::JSONOutputArchive jsonOutputArchive(blogStream);
            jsonOutputArchive(*blog);
        }
    } else {
        printDatabaseError();
    }
    sqlite3_finalize(stmt);
    return blogStream.str();
}

std::string BlogManager::retrieveFormattedBlogForRequest(const std::string &requestString) {
    int blogId = convertIdToInt(requestString.substr(1,requestString.length()-1));
    std::stringstream blogStream;
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(mDatabase, "SELECT title, datetime, content FROM blogs WHERE id = ?;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_bind_int(stmt, 1, blogId) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        std::unique_ptr blog = std::make_unique<Blog>(
                reinterpret_cast<char const *>(sqlite3_column_text(stmt, 0)),
                reinterpret_cast<char const *>(sqlite3_column_text(stmt, 1)),
                reinterpret_cast<char const *>(sqlite3_column_text(stmt, 2)),
                getFormattedLinkToPreviousBlog(blogId),
                getFormattedLinkToNextBlog(blogId));
        cereal::JSONOutputArchive jsonOutputArchive(blogStream);
        jsonOutputArchive(*blog);
    } else {
        printDatabaseError();
    }
    sqlite3_finalize(stmt);
    return blogStream.str();
}

void BlogManager::removeBlog(const std::string &blogToRemove) {
    std::string idAttribute = "id=";
    std::string csrfAttribute = "\n_csrf=";
    unsigned long idLoc = blogToRemove.find(idAttribute);
    unsigned long tokenLoc = blogToRemove.find(csrfAttribute);
    std::string blogIdStr = blogToRemove.substr(idLoc + idAttribute.length(), (tokenLoc - (idLoc + idAttribute.length())) - 1);
    int blogId = convertIdToInt(blogIdStr);
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(mDatabase, "DELETE FROM blogs WHERE id = ?;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_bind_int(stmt, 1, blogId) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_step(stmt) != SQLITE_DONE){
        printDatabaseError();
    }
    sqlite3_finalize(stmt);
    updateBlogIndex();
}

int BlogManager::convertIdToInt(const std::string stringToConvert) {
    return std::stoi(stringToConvert);
}

bool BlogManager::checkForBlogByTitle(const std::string &blogTitle) {
    sqlite3_stmt *stmt;
    bool result = false;
    if(sqlite3_prepare_v2(mDatabase, "SELECT * FROM blogs WHERE title = ?;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_bind_text(stmt, 1, blogTitle.c_str(), static_cast<int>(blogTitle.length()), SQLITE_TRANSIENT) != SQLITE_OK){
        printDatabaseError();
    }
    if(sqlite3_step(stmt) == SQLITE_ROW){
        std::cout << "Duplicate" << std::endl;
        result = true;
    }
    sqlite3_finalize(stmt);
    return result;
}

bool BlogManager::checkForValidBlogRequest(const std::string &requestedBlog) {
    return std::regex_match(requestedBlog, mGetBlogIdRegexFormula);
}

void BlogManager::updateBlogIndex() {
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(mDatabase, "SELECT id, title FROM blogs ORDER BY id;", -1, &stmt, nullptr) != SQLITE_OK){
        printDatabaseError();
    }
    std::map<int,std::string> blogIndex;
    while(sqlite3_step(stmt) == SQLITE_ROW){
        blogIndex.emplace(sqlite3_column_int(stmt,0), reinterpret_cast<char const *>(sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);
    std::stringstream stream;
    {
        cereal::JSONOutputArchive jsonOutputArchive(stream);
        jsonOutputArchive(blogIndex);
    }
    mUpdateBlogIndexMutex.lock();
    mJSONifiedIndex = stream.str();
    mUpdateBlogIndexMutex.unlock();
}

std::string BlogManager::retrieveBlogIndex() {
    mUpdateBlogIndexMutex.lock_shared();
    std::string blogIndex = mJSONifiedIndex;
    mUpdateBlogIndexMutex.unlock_shared();
    return blogIndex;
}

void BlogManager::initializeIndexFromDatabase() {
    updateBlogIndex();
}
