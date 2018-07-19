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
    std::string retrieveFormattedBlogForRequest(const std::string &requestString);
    void createBlogFromSubmission(const std::string &blogContent);
    std::string retrieveMostRecentBlog();
    void removeBlog(const std::string &blogToRemove);
    void writeBlogIndexPage(const std::string &pageDir);
    void lockRead();
    void unlockRead();
private:
    void printDatabaseError();
    bool checkForBlogByTitle(const std::string &blogTitle);
    int convertIdToInt(std::string stringToConvert);
    void formatIndexPage(sqlite3_stmt *stmt, std::stringstream &blogIndex) const;
    int getFormattedLinkToPreviousBlog(const int &id);
    int getFormattedLinkToNextBlog(const int &id);
    sqlite3 *&mDatabase;
    const std::regex mGetBlogIdRegexFormula;
    std::shared_mutex mWritingBlogsFileMutex;
};

#endif //SERVER_BLOGMANAGER_H
