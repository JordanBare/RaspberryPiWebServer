
#include <iostream>
#include "Session.h"
#include "Listener.h"
#include "Server.h"
#include <boost/asio/io_context.hpp>

Blog createBlog();

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

    boost::asio::io_context ioContext{numThreads};
    Server server(port, numThreads, ioContext, rootDir);
    server.run();
}