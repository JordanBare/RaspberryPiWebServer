//
// Created by Jordan Bare on 6/12/18.
//

#include <fstream>
#include "Server.h"
#include <cereal/archives/portable_binary.hpp>
#include <iostream>

Server::Server(unsigned short port,
               boost::asio::io_context &ioContext,
               std::string rootDir): mIOContext(ioContext),
                                     mFolderRoots({rootDir + "//pages//", rootDir + "//blogs//"}),
                                     mListener(std::make_shared<Listener>(mIOContext,
                                                                        boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address("0::0"),
                                                                                                       port},
                                                                        mIndexMap, mFolderRoots)){
    readBlogIndexFile();
}

void Server::readBlogIndexFile() {
    std::ifstream file(mFolderRoots[1] + "blogindex.txt");
    if(file.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(file);
        mIndexMapMutex.lock();
        inputArchive(mIndexMap);
        mIndexMapMutex.unlock();
        file.close();
    }
}

void Server::run(unsigned short numThreads) {
    (*mListener).run();
    mWorkerThreads.reserve(numThreads);
    for(unsigned short i = 0; i < numThreads; ++i){
        mWorkerThreads.emplace_back([this]{
            mIOContext.run();
        });
    }
    displayMenu();
}

void Server::displayMenu() {
    char option;
    bool accessOptions = true;
    const std::string options = "\nOptions:\nt : Terminate program\ns : Sessions held\nl : List blogs\nc : Create blog\nd : Destroy blog";
    while(accessOptions){
        std::cout << options << std::endl;
        std::cin >> option;
        switch(option){
            case 't': {
                accessOptions = false;
                break;
            }
            case 's': {
                std::cout << "Sessions held: " << (*mListener).reportSessionsHeld() << std::endl;
                break;
            }

            case 'c': {
                createBlogFiles();
                break;
            }
            case 'l': {
                for(auto const& it : mIndexMap) {
                    std::cout << it.first << ": " << it.second << std::endl;
                }
                break;
            }
            case 'd': {
                std::string blogNumber;
                std::cout << "Enter the number of the blog to be destroyed (Press n to stop): " << std::endl;
                std::cin >> blogNumber;
                if(blogNumber == "n"){
                    break;
                }
                std::cout << blogNumber << std::endl;
                destroyBlog(blogNumber);
                break;
            }
            default: {
                accessOptions = false;
                break;
            }
        }
    }
}

void Server::createBlogFiles() {

    Blog blog = createBlogFromInfo();
    unsigned short newBlogIndex = mIndexMap.size() + 1;
    std::ofstream blogFile(mFolderRoots[1] + std::to_string(newBlogIndex) + ".txt");
    if(blogFile.is_open()){
        cereal::PortableBinaryOutputArchive blogFileBinaryOutputArchive(blogFile);
        blogFileBinaryOutputArchive(blog);
        blogFile.flush();
        blogFile.close();
    }

    mIndexMapMutex.lock();
    mIndexMap.emplace(newBlogIndex, blog.getTitle());
    mIndexMapMutex.unlock();
    writeBlogIndexFile();
    //writeBlogListPageFile();
}

Blog Server::createBlogFromInfo() const {
    std::string title;
    std::cout << "Enter the title: " << std::endl;
    std::cin.ignore();
    getline(std::cin, title);
    std::string content;
    std::cout << "Enter the content: " << std::endl;
    getline(std::cin, content);
    Blog blog(title, content);
    return blog;
}

Server::~Server() {
    mIOContext.stop();
    for(auto &thread: mWorkerThreads){
        thread.join();
    }
}

void Server::destroyBlog(std::string blogToDestroy) {
    unsigned short blogNumber = boost::lexical_cast<unsigned short>(blogToDestroy);
    auto it = mIndexMap.find(blogNumber);
    mIndexMapMutex.lock();
    mIndexMap.erase(it);
    mIndexMapMutex.unlock();
    writeBlogIndexFile();
    std::string pathToFileToDelete(mFolderRoots[1] + std::to_string(blogNumber) + ".txt");
    std::remove(pathToFileToDelete.c_str());
}

void Server::writeBlogIndexFile() {
    std::ofstream indexFile(mFolderRoots[1] + "blogIndex.txt");
    if(indexFile.is_open()){
        cereal::PortableBinaryOutputArchive indexFileBinaryOutputArchive(indexFile);
        mIndexMapMutex.lock();
        indexFileBinaryOutputArchive(mIndexMap);
        mIndexMapMutex.unlock();
        indexFile.flush();
        indexFile.close();
    }
}

void Server::writeBlogListPageFile() {
    std::ifstream bloglistTemplateFile(mFolderRoots[0] + "bloglisttemplate.html");
    std::string blogListPage;
    if(bloglistTemplateFile.is_open()) {
        std::stringstream stringstream;
        stringstream << bloglistTemplateFile.rdbuf();
        blogListPage = stringstream.str();
        bloglistTemplateFile.close();
    }
    std::stringstream blogListEntryStream;
    std::map<unsigned short, std::string>::iterator it;
    for (it = mIndexMap.begin(); it != mIndexMap.end(); ++it) {
        blogListEntryStream << "<tr>";
        for(unsigned short i = 0; i != 3 || it != mIndexMap.end(); ++i, ++it){
            blogListEntryStream << "<td>" <<  "<a href=\"/blog" << it->first << "\">" << it->second << "</a></td>";
        }
        std::cout << "should be finished" << std::endl;
        blogListEntryStream << "</tr>";
    }
    std::string stringToFind = "bloglistentry";
    size_t replacementPosition = blogListPage.find(stringToFind);
    blogListPage.replace(replacementPosition, stringToFind.length(), blogListEntryStream.str());

    std::ofstream blogListFile(mFolderRoots[0] + "bloglist.html");
    if(blogListFile.is_open()){
        mIndexMapMutex.lock();
        blogListFile << blogListPage;
        mIndexMapMutex.unlock();
        blogListFile.flush();
        blogListFile.close();
    }


}

