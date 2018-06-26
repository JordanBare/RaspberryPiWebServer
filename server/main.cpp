
#include <iostream>
#include "Session.h"
#include "Listener.h"
#include "Server.h"
//#include <boost/asio/ssl/context.hpp>
#include <boost/asio/io_context.hpp>

//void loadCertificate(std::string &rootDir, boost::asio::ssl::context &context);

int main() {
    unsigned short port;
    unsigned short numThreads;
    std::string rootDir;

    std::cout << "HTTP Server Activated\nPort number?" << std::endl;
    //std::cin >> port;
    port = 7000;
    std::cout << "Number of service threads?" << std::endl;
    //std::cin >> numThreads;
    numThreads = 1;
    std::cout << "Root directory? (Use \".\" for same directory as executable)" << std::endl;
    //std::cin >> rootDir;
    rootDir = "..";

    /*
    boost::asio::ssl::context context{boost::asio::ssl::context::tlsv12_server};

    loadCertificate(rootDir,context);
     */

    boost::asio::io_context ioContext{numThreads};
    Server server(port, ioContext, rootDir);
    server.run(numThreads);
}

/*
void loadCertificate(std::string &rootDir, boost::asio::ssl::context &context) {
    std::string password;
    std::cout << "Please enter the password for the certs: " << std::endl;
    std::cin >> password;
    context.set_password_callback(password);
    context.use_certificate_chain_file(rootDir + "/cert/server.key");
    context.use_private_key_file(rootDir + "/cer/server.pem", boost::asio::ssl::context::pem);
    context.use_tmp_dh_file(rootDir + "/cert/");
}
 */