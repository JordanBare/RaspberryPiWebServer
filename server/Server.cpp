//
// Created by Jordan Bare on 6/12/18.
//

#include <fstream>
#include "Server.h"
#include <cereal/archives/portable_binary.hpp>
#include <iostream>

Server::Server(unsigned short port,
               unsigned short numThreads,
               std::string rootDir): mIOContext(numThreads),
                                     mSSLContext(boost::asio::ssl::context::tlsv12_server),
                                     mRootDir(rootDir),
                                     mListener(std::make_shared<Listener>(mIOContext,
                                                                          mSSLContext,
                                                                          boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address("0::0"), port},
                                                                        mIndexMap,
                                                                          rootDir)){

    mSSLContext.set_options(boost::asio::ssl::context::default_workarounds |
                            boost::asio::ssl::context::no_sslv2 |
                            boost::asio::ssl::context::no_sslv3 |
                            boost::asio::ssl::context::no_tlsv1 |
                            boost::asio::ssl::context::no_tlsv1_1 |
                            boost::asio::ssl::context::single_dh_use);
    loadCertificate();
    //readBlogIndexFile();
}

/*
void Server::readBlogIndexFile() {
    std::ifstream file(mFolderRoots[1] + "blogindex.txt");
    if(file.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(file);
        inputArchive(mIndexMap);
        file.close();
    }
}
 */

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
                //createBlogFiles();
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
                //destroyBlog(blogNumber);
                break;
            }
            default: {
                accessOptions = false;
                break;
            }
        }
    }
}

/*
void Server::createBlogFiles() {

    Blog blog = createBlogFromInfo();
    unsigned short newBlogIndex = mIndexMap.size() + 1;
    std::ofstream blogFile(mFolderRoots[1] + std::to_string(newBlogIndex) + ".txt");
    if(blogFile.is_open()){
        cereal::PortableBinaryOutputArchive blogFileBinaryOutputArchive(blogFile);
        blogFileBinaryOutputArchive(blog);
        blogFile.close();
    }
    mIndexMap.emplace(newBlogIndex, blog.getTitle());
    writeBlogIndexFile();
    writeBlogListPageFile();
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
*/

Server::~Server() {
    mIOContext.stop();
    for(auto &thread: mWorkerThreads){
        thread.join();
    }
}

void Server::loadCertificate() {
    std::string password;
    std::cout << "Please enter the password for the certs: " << std::endl;
    password = "password";
    mSSLContext.set_password_callback(std::bind(&Server::get_Password,this));
    mSSLContext.use_certificate_chain_file(mRootDir + "/cert/server.crt");
    mSSLContext.use_private_key_file(mRootDir + "/cert/server.key", boost::asio::ssl::context::pem);
    mSSLContext.use_tmp_dh_file(mRootDir + "/cert/dh2048.pem");
}

std::string Server::get_Password() {
    return "password";
}


/*
void Server::destroyBlog(std::string blogToDestroy) {
    unsigned short blogNumber = boost::lexical_cast<unsigned short>(blogToDestroy);
    auto it = mIndexMap.find(blogNumber);
    mIndexMap.erase(it);
    writeBlogIndexFile();
    std::string pathToFileToDelete(mFolderRoots[1] + std::to_string(blogNumber) + ".txt");
    std::remove(pathToFileToDelete.c_str());
    writeBlogListPageFile();
}

void Server::writeBlogIndexFile() {
    std::ofstream indexFile(mFolderRoots[1] + "blogIndex.txt");
    if(indexFile.is_open()){
        cereal::PortableBinaryOutputArchive indexFileBinaryOutputArchive(indexFile);
        indexFileBinaryOutputArchive(mIndexMap);
        indexFile.close();
    }
}

void Server::writeBlogListPageFile() {
    std::stringstream blogListStream;
    unsigned short blogCount = 1;
    blogListStream << "<br><br><table id=\"blogs\"><tr>";
    std::map<unsigned short, std::string>::iterator it;
    for (it = mIndexMap.begin(); it != mIndexMap.end(); ++it) {
        blogListStream << "<td><button onclick=\"loadDoc('/blog" << it->first << "')\">" << it->second << "</button></td>";
        if(blogCount % 3 == 0){
            blogListStream << "</tr><tr>";
        }
        blogCount++;
    }
    blogListStream << "</tr></table>";

    std::ofstream blogListFile(mFolderRoots[0] + "blogs.html");
    if(blogListFile.is_open()){
        blogListFile << blogListStream.str();
        blogListFile.close();
    }
}
 */

