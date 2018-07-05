//
// Created by Jordan Bare on 6/4/18.
//

#include <fstream>
#include "Listener.h"
#include "Session.h"

Listener::Listener(boost::asio::io_context &ioc,
                   boost::asio::ssl::context &sslContext,
                   boost::asio::ip::tcp::endpoint endpoint,
                   std::map<unsigned short, std::string> &indexMap,
                   std::string rootDir):
        mAcceptor(ioc),
        mSessionSocket(ioc),
        mIOContext(ioc),
        mSSLContext(sslContext),
        mIndexMap(indexMap),
        mFolderRoots({rootDir + "//pages//", rootDir + "//blogs//"}),
        mTotalSessions(0) {

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

void Listener::run() {
    if(!mAcceptor.is_open()){
        return;
    }
    doAccept();
}

void Listener::doAccept() {
    mAcceptor.async_accept(mSessionSocket,
                           std::bind(&Listener::onAccept,
                                     shared_from_this(),
                                     std::placeholders::_1));
}

void Listener::onAccept(boost::system::error_code ec) {
    if(ec){
        printErrorCode(ec);
        return;
    }
    std::make_shared<Session>(mSSLContext, std::move(mSessionSocket), mCSRFSet, mIndexMap, mFolderRoots)->run();
    mTotalSessions++;
    doAccept();
}

void Listener::printErrorCode(boost::system::error_code &ec) {
    std::cout << "Error code: "
                 << ec.value()
                 << " | Message : "
                    << ec.message()
                    << std::endl;
}

unsigned int Listener::reportSessionsHeld() {
    return mTotalSessions;
}


