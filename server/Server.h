//
// Created by Jordan Bare on 6/12/18.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H


#include <thread>
#include "SessionManager.h"
#include <boost/asio/ssl/context.hpp>

class Server {
public:
    Server(unsigned short port, unsigned short numThreads, std::string rootDir);
    ~Server();
    void run(unsigned short numThreads);
private:
    void displayMenu();
    void loadCertificate();
    std::string get_Password();

    boost::asio::io_context mIOContext;
    boost::asio::ssl::context mSSLContext;
    std::string mRootDir;
    std::shared_ptr<SessionManager> mSessionManager;
    std::vector<std::thread> mWorkerThreads;
};

#endif //SERVER_SERVER_H
