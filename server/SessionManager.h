//
// Created by Jordan Bare on 6/4/18.
//

#ifndef SERVER_LISTENER_H
#define SERVER_LISTENER_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <sqlite3.h>
#include "CSRFManager.h"
#include "BlogManager.h"
#include "CredentialsManager.h"

class SessionManager : public std::enable_shared_from_this<SessionManager> {
public:
    SessionManager(boost::asio::io_context& ioc, boost::asio::ssl::context &sslContext, boost::asio::ip::tcp::endpoint endpoint, sqlite3 *&database, const std::string &rootDir);
    void run();
    unsigned int reportSessionsHeld();
private:
    void doAccept();
    void onAccept(boost::system::error_code ec);
    void printErrorCode(boost::system::error_code &ec);

    boost::asio::ip::tcp::acceptor mAcceptor;
    boost::asio::ip::tcp::socket mSessionSocket;
    boost::asio::io_context& mIOContext;
    boost::asio::ssl::context& mSSLContext;
    const std::string mPageRoot;
    std::unique_ptr<CSRFManager> mCSRFManager;
    std::unique_ptr<BlogManager> mBlogManager;
    std::unique_ptr<CredentialsManager> mCredentialsManager;
    std::atomic<unsigned int> mTotalSessions;
};

#endif //SERVER_LISTENER_H
