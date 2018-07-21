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
    void initializeIndexFromDatabase();
    bool checkForValidBlogRequest(const std::string &requestedBlog);
    std::string retrieveFormattedBlogForRequest(const std::string &requestString);
    void createBlogFromSubmission(const std::string &blogContent);
    std::string retrieveMostRecentBlog();
    void removeBlog(const std::string &blogToRemove);
    std::string retrieveBlogIndex();
private:
    void printDatabaseError();
    bool checkForBlogByTitle(const std::string &blogTitle);
    int convertIdToInt(std::string stringToConvert);
    int getFormattedLinkToPreviousBlog(const int &id);
    int getFormattedLinkToNextBlog(const int &id);
    void updateBlogIndex();
    sqlite3 *&mDatabase;
    const std::regex mGetBlogIdRegexFormula;
    std::string mJSONifiedIndex;
    std::shared_mutex mUpdateBlogIndexMutex;
};

#endif //SERVER_BLOGMANAGER_H
