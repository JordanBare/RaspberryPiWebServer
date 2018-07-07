//
// Created by Jordan Bare on 7/6/18.
//

#include <fstream>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/map.hpp>
#include "BlogManager.h"

BlogManager::BlogManager(const std::vector<std::string> &folderRoots): mBlogDir(folderRoots[1]), mRegexFormula("^/[1-9][0-9]{0,4}") {

    readBlogIndexFile();
}

void BlogManager::readBlogIndexFile() {
    std::ifstream file(mBlogDir + "blogindex.txt");
    if(file.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(file);
        inputArchive(mBlogIndex);
        file.close();
    }
}

std::unique_ptr<Blog> BlogManager::readBlogFromFile(const std::string &resourceFilePath) {
    std::unique_ptr<Blog> blog = std::make_unique<Blog>();
    std::ifstream file(resourceFilePath);
    if(file.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(file);
        inputArchive(*blog);
        file.close();
    }
    return blog;
}

std::unique_ptr<Blog> BlogManager::createBlogFromSubmission(const std::string &blogContent) {


    unsigned long titleLoc = blogContent.find("&title=");
    unsigned long blogLoc = blogContent.find("&blog=");
    std::string usr = blogContent.substr(5, blogLoc-5);
    std::string title;
    std::string content;
    std::unique_ptr<Blog> blog = std::make_unique<Blog>();
    return blog;
}

void BlogManager::writeBlogFile(std::unique_ptr<Blog> blog) {
    std::ofstream blogFile(mBlogDir + ".txt");
    if(blogFile.is_open()){
        cereal::PortableBinaryOutputArchive blogFileBinaryOutputArchive(blogFile);
        blogFileBinaryOutputArchive(*blog);
        blogFile.close();
    }
}

std::string BlogManager::retrieveFormattedBlog() {

    return std::string();
}

bool BlogManager::checkForBlog(const std::string &requestString) {
    if(std::regex_match(requestString, mRegexFormula)){
        std::string blogRequested = requestString.substr(1,requestString.length()-1);
        mMapAccessMutex.lock();
        bool blogFound = mBlogIndex.count(1) == 1;
        mMapAccessMutex.unlock();
        if(blogFound){
            return true;
        }
    }
    return false;
}
