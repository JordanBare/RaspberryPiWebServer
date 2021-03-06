//
// Created by Jordan Bare on 6/12/18.
//

#include <fstream>
#include <iostream>
#include "Server.h"

Server::Server(unsigned short port,
               unsigned short numThreads,
               std::string rootDir): mIOContext(numThreads),
                                     mSSLContext(boost::asio::ssl::context::tlsv12_server),
                                     mRootDir(std::move(rootDir)),
                                     mSessionManager(std::make_shared<SessionManager>(mIOContext,
                                                                                      mSSLContext,
                                                                                      boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address("0::0"), port},
                                                                                      mDatabase,
                                                                                      mRootDir)){
    mSSLContext.set_options(boost::asio::ssl::context::default_workarounds |
                            boost::asio::ssl::context::no_sslv2 |
                            boost::asio::ssl::context::no_sslv3 |
                            boost::asio::ssl::context::no_tlsv1 |
                            boost::asio::ssl::context::no_tlsv1_1 |
                            boost::asio::ssl::context::single_dh_use);
    loadCertificate();
    std::string dbDir(mRootDir + "//database//webDB.db");
    if(sqlite3_open(dbDir.c_str(), &mDatabase)){
        std::cout << "No database file!" << std::endl;
    }
}

void Server::run(unsigned short numThreads) {
    (*mSessionManager).run();
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
    const std::string options = "\nOptions:\nt : Terminate program\ns : Sessions held";
    while(accessOptions){
        std::cout << options << std::endl;
        std::cin >> option;
        switch(option){
            case 't': {
                accessOptions = false;
                break;
            }
            case 's': {
                std::cout << "Sessions held: " << (*mSessionManager).reportSessionsHeld() << std::endl;
                break;
            }
            default: {
                accessOptions = false;
                break;
            }
        }
    }
}

Server::~Server() {
    mIOContext.stop();
    for(auto &thread: mWorkerThreads){
        thread.join();
    }
    sqlite3_close(mDatabase);
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
