//
// Created by Jordan Bare on 6/4/18.
//

#include <fstream>
#include "SessionManager.h"
#include "Session.h"

SessionManager::SessionManager(boost::asio::io_context &ioc,
                   boost::asio::ssl::context &sslContext,
                   boost::asio::ip::tcp::endpoint endpoint,
                   std::string rootDir):
        mAcceptor(ioc),
        mSessionSocket(ioc),
        mIOContext(ioc),
        mSSLContext(sslContext),
        mFolderRoots({rootDir + "//pages//", rootDir + "//blogs//", rootDir + "//creds//"}),
        mTotalSessions(0),
        mCSRFManager(std::make_unique<CSRFManager>()),
        mBlogManager(std::make_unique<BlogManager>(mFolderRoots)),
        mCredentialsManager(std::make_unique<CredentialsManager>(mFolderRoots)){

    boost::system::error_code ec;
    mAcceptor.open(endpoint.protocol(), ec);

    if(ec){
        printErrorCode(ec);
        return;
    }
    mAcceptor.set_option(boost::asio::socket_base::reuse_address(true));

    if(ec){
        printErrorCode(ec);
        return;
    }
    mAcceptor.bind(endpoint, ec);

    if(ec){
        printErrorCode(ec);
        return;
    }
    mAcceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

    if(ec){
        printErrorCode(ec);
        return;
    }
}

void SessionManager::run() {
    if(!mAcceptor.is_open()){
        return;
    }
    doAccept();
}

void SessionManager::doAccept() {
    mAcceptor.async_accept(mSessionSocket,
                           std::bind(&SessionManager::onAccept,
                                     shared_from_this(),
                                     std::placeholders::_1));
}

void SessionManager::onAccept(boost::system::error_code ec) {
    if(ec){
        printErrorCode(ec);
        return;
    }
    std::make_shared<Session>(mSSLContext, std::move(mSessionSocket), mCSRFManager, mBlogManager, mCredentialsManager, mFolderRoots)->run();
    mTotalSessions++;
    doAccept();
}

void SessionManager::printErrorCode(boost::system::error_code &ec) {
    std::cout << "Error code: " << ec.value() << " | Message : " << ec.message() << std::endl;
}

unsigned int SessionManager::reportSessionsHeld() {
    return mTotalSessions;
}


