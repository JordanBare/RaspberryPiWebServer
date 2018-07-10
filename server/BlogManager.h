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
    bool checkForBlog(const std::string &requestString);
    std::string retrieveFormattedBlog();
    std::unique_ptr<Blog> createBlogFromSubmission(const std::string &blogContent);
private:
    void readBlogIndexFile();
    std::unique_ptr<Blog> readBlogFromFile(const std::string &resourceFilePath);
    void writeBlogFile(std::unique_ptr<Blog> blog);
    friend class cereal::access;
    template <class Archive> void serialize(Archive &ar) {
        ar(mBlogIndex);
    }
    std::map<unsigned short,std::string> mBlogIndex;
    std::mutex mMapAccessMutex;
    const std::string mBlogDir;
    const std::regex mRegexFormula;
};


#endif //SERVER_BLOGMANAGER_H
