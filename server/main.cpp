
#include <iostream>
#include "Session.h"
#include "Listener.h"
#include "Server.h"

int main() {
    unsigned short port;
    unsigned short numThreads;
    std::string rootDir;

    std::cout << "HTTPS Server Activated\nPort number?" << std::endl;
    //std::cin >> port;
    port = 8080;
    std::cout << "Number of service threads?" << std::endl;
    //std::cin >> numThreads;
    numThreads = 1;
    std::cout << "Root directory? (Use \".\" for same directory as executable)" << std::endl;
    //std::cin >> rootDir;
    rootDir = "..";

    //boost::asio::io_context ioContext{numThreads};
    //Server server(port, ioContext, rootDir);
    Server server(port, numThreads, rootDir);
    server.run(numThreads);
}

