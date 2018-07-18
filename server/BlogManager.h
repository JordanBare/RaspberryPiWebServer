//
// Created by Jordan Bare on 7/6/18.
//

#ifndef SERVER_BLOGMANAGER_H
#define SERVER_BLOGMANAGER_H

#include <regex>
#include <shared_mutex>
#include <sqlite3.h>

class BlogManager {
public:
    explicit BlogManager(sqlite3 *&database);
    bool checkForValidBlogRequest(const std::string &requestedBlog);
    std::string retrieveFormattedBlog(const std::string &requestString);
    void createBlogFromSubmission(const std::string &blogContent);
    void removeBlog(const std::string &blogToRemove);
    void writeBlogIndexPage(const std::string &pageDir);
    void lockRead();
    void unlockRead();
private:
    void printDatabaseError();
    bool checkForBlogByTitle(const std::string &blogTitle);
    int convertIdToInt(std::string stringToConvert);
    void formatIndexPage(sqlite3_stmt *stmt, std::stringstream &blogIndex) const;
    sqlite3 *&mDatabase;
    const std::regex mGetBlogIdRegexFormula;
    std::shared_mutex mWritingBlogsFileMutex;
};

#endif //SERVER_BLOGMANAGER_H
