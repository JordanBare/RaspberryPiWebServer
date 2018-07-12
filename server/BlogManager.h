//
// Created by Jordan Bare on 7/6/18.
//

#ifndef SERVER_BLOGMANAGER_H
#define SERVER_BLOGMANAGER_H

#include <mutex>
#include <regex>
#include "Blog.h"

class BlogManager {
public:
    explicit BlogManager(const std::vector<std::string> &folderRoots);
    bool checkForRequestedBlog(const std::string &requestString);
    std::string retrieveFormattedBlog(const std::string &requestString);
    void createBlogFromSubmission(const std::string &blogContent);
    void removeBlog(const std::string &blogToRemove);
private:
    void readBlogIndexFile();
    std::unique_ptr<Blog> readBlogFromFile(const std::string &resourceFilePath);
    void writeBlogFile(std::unique_ptr<Blog> &blog);
    bool checkForBlog(const std::string &blogRequested);
    void writeBlogIndexFile();
    friend class cereal::access;
    template <class Archive> void serialize(Archive &ar) {
        ar(mBlogIndex);
    }
    std::map<std::string,std::string> mBlogIndex;
    std::mutex mMapAccessMutex;
    const std::string mBlogDir;
    const std::regex mRegexFormula;
};

#endif //SERVER_BLOGMANAGER_H
