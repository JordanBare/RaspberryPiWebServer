//
// Created by Jordan Bare on 6/4/18.
//

#ifndef SERVER_LISTENER_H
#define SERVER_LISTENER_H


#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint, std::shared_ptr<std::string> &pageRoot);
    void run();
    void doAccept();
    void onAccept(boost::system::error_code ec);
    unsigned int reportSessionsHeld();
private:
    void printErrorCode(boost::system::error_code &ec);

    boost::asio::ip::tcp::acceptor mAcceptor;
    std::shared_ptr<std::string> mPageRoot;
    boost::asio::ip::tcp::socket mSessionSocket;
    std::atomic<unsigned int> mTotalSessions;
};


#endif //SERVER_LISTENER_H
