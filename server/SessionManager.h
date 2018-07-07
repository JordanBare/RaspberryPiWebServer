//
// Created by Jordan Bare on 6/4/18.
//

#ifndef SERVER_LISTENER_H
#define SERVER_LISTENER_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <set>
#include "Blog.h"

class SessionManager : public std::enable_shared_from_this<SessionManager> {
public:
    SessionManager(boost::asio::io_context& ioc, boost::asio::ssl::context &sslContext, boost::asio::ip::tcp::endpoint endpoint, std::map<unsigned short, std::string> &indexMap, std::string rootDir);
    void run();
    void doAccept();
    void onAccept(boost::system::error_code ec);
    unsigned int reportSessionsHeld();
private:
    void printErrorCode(boost::system::error_code &ec);

    boost::asio::ip::tcp::acceptor mAcceptor;
    boost::asio::ip::tcp::socket mSessionSocket;
    boost::asio::io_context& mIOContext;
    boost::asio::ssl::context& mSSLContext;
    std::map<unsigned short, std::string> &mIndexMap;
    std::set<std::string> mCSRFSet;
    const std::vector<std::string> mFolderRoots;
    std::atomic<unsigned int> mTotalSessions;
};


#endif //SERVER_LISTENER_H
