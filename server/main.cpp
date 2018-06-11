
#include <iostream>
#include "Session.h"
#include "Listener.h"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <thread>

int main() {

    unsigned short port;
    unsigned short numThreads;

    std::cout << "HTTP Server Activated\nPort number?" << std::endl;
    std::cin >> port;
    std::cout << "Number of service threads?" << std::endl;
    std::cin >> numThreads;

    std::shared_ptr<std::string> pageRoot = std::make_shared<std::string>(".//pages/");
    const boost::asio::ip::address address = boost::asio::ip::make_address("0::0");
    boost::asio::io_context ioContext{numThreads};

    std::shared_ptr<Listener> listener(std::make_shared<Listener>(ioContext,
                                                                  boost::asio::ip::tcp::endpoint{address, port},
                                                                  pageRoot));
    (*listener).run();

    std::vector<std::thread> workerThreads;
    workerThreads.reserve(numThreads);
    for(unsigned short i = 0; i < numThreads; ++i){
        workerThreads.emplace_back([&ioContext]{
           ioContext.run();
        });
    }
    char option;
    bool accessOptions = true;
    while(accessOptions){
        std::cout << "\nOptions:\nt : Terminate program\ns : Sessions held" << std::endl;
        std::cin >> option;
        switch(option){
            case 't':
                accessOptions = false;
                break;
            case 's':
                std::cout << "Sessions held: " << (*listener).reportSessionsHeld() << std::endl;
                break;
            default:
                accessOptions = false;
                break;
        }
    }

    ioContext.stop();

    for(auto &thread: workerThreads){
        thread.join();
    }

    return 0;
}