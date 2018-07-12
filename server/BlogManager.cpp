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
    std::ifstream blogIndexFile(mBlogDir + "blogindex.txt");
    if(blogIndexFile.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(blogIndexFile);
        inputArchive(mBlogIndex);
        blogIndexFile.close();
    } else {
        writeBlogIndexFile();
    }
}

void BlogManager::writeBlogIndexFile() {
    std::ofstream blogIndexFile(mBlogDir + "blogindex.txt");
    if(blogIndexFile.is_open()){
        cereal::PortableBinaryOutputArchive outputArchive(blogIndexFile);
        mMapAccessMutex.lock();
        outputArchive(mBlogIndex);
        mMapAccessMutex.unlock();
        blogIndexFile.close();
    }
}

void BlogManager::createBlogFromSubmission(const std::string &blogContent) {

    unsigned long titleLoc = blogContent.find("title=");
    unsigned long blogLoc = blogContent.find("&blog=");
    unsigned long tokenLoc = blogContent.find("&_csrf=");
    std::string title = blogContent.substr(titleLoc + 6, blogLoc - 6);
    std::string content = blogContent.substr(blogLoc + 6, tokenLoc - (blogLoc + 6));
    std::cout << blogContent << std::endl;
    std::unique_ptr<Blog> blog = std::make_unique<Blog>(title,content);
    std::cout << "Title: " << title << "\nBlog: " << content << std::endl;

    if(!checkForBlog(blog->getTitle())){
        writeBlogFile(blog);
        mMapAccessMutex.lock();
        mBlogIndex.emplace();
        mMapAccessMutex.unlock();
        writeBlogIndexFile();
    }
}

void BlogManager::writeBlogFile(std::unique_ptr<Blog> &blog) {
    std::ofstream blogFile(mBlogDir + blog->getTitle() + ".txt");
    if(blogFile.is_open()){
        cereal::PortableBinaryOutputArchive outputArchive(blogFile);
        outputArchive(*blog);
        blogFile.close();
    }
}

std::string BlogManager::retrieveFormattedBlog(const std::string &requestString) {
    std::string blogRequested = requestString.substr(1,requestString.length()-1);
    std::unique_ptr<Blog> requestedBlog = readBlogFromFile(blogRequested);
    return requestedBlog->getBlogPage();
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

bool BlogManager::checkForRequestedBlog(const std::string &requestString) {
    if(std::regex_match(requestString, mRegexFormula)){
        std::string blogRequested = requestString.substr(1,requestString.length()-1);
        return checkForBlog(blogRequested);
    }
    return false;
}

bool BlogManager::checkForBlog(const std::string &blogRequested)  {
    mMapAccessMutex.lock();
    bool blogFound = mBlogIndex.count(blogRequested) == 1;
    mMapAccessMutex.unlock();
    return blogFound;
}

void BlogManager::removeBlog(const std::string &blogToRemove) {
    unsigned long idLoc = blogToRemove.find("id=");
    unsigned long tokenLoc = blogToRemove.find("&_csrf=");
    std::string blogId = blogToRemove.substr(idLoc + 3, tokenLoc - (idLoc + 3));
    if(checkForBlog(blogId)){
        mMapAccessMutex.lock();
        mBlogIndex.erase(blogId);
        mMapAccessMutex.unlock();
        std::string blogFileDir = mBlogDir + blogId + ".txt";
        std::remove(mBlogDir.c_str());
    }
}

