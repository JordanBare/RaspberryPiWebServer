//
// Created by Jordan Bare on 6/12/18.
//

#include <fstream>
#include "Server.h"
#include <cereal/archives/portable_binary.hpp>
#include <iostream>

Server::Server(unsigned short port,
               unsigned short numThreads,
               boost::asio::io_context &ioContext,
               std::string rootDir): mNumThreads(numThreads),
                                     mIOContext(ioContext),
                                     mFolderRoots({rootDir + "//pages//", rootDir + "//blogs//"}),
                                     mListener(std::make_shared<Listener>(mIOContext,
                                                                        boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address("0::0"),
                                                                                                       port},
                                                                        mIndexMap, mFolderRoots)){
    std::ifstream file(mFolderRoots[1] + "blogIndex.txt");
    if(file.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(file);
        mIndexMapMutex.lock();
        inputArchive(mIndexMap);
        mIndexMapMutex.unlock();
        file.close();
    }
    mWorkerThreads.reserve(mNumThreads);
}

void Server::run() {

    (*mListener).run();
    for(unsigned short i = 0; i < mNumThreads; ++i){
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
                createBlog();
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

void Server::createBlog() {
    std::string title;
    std::cout << "Enter the title: " << std::endl;
    std::cin.ignore();
    getline(std::cin,title);
    std::string content;
    std::cout << "Enter the content: " << std::endl;
    getline(std::cin,content);
    Blog blog(title, content);
    std::cout << blog.getContent() << std::endl;

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
    std::string pathToFileToDeletion(mFolderRoots[1] + std::to_string(blogNumber) + ".txt");
    std::remove(pathToFileToDeletion.c_str());
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

