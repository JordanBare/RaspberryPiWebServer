//
// Created by Jordan Bare on 6/4/18.
//

#include <ios>
#include <fstream>
#include "Session.h"


Session::Session(boost::asio::ip::tcp::socket socket, std::shared_ptr<std::string> &pageRoot): mSocket(std::move(socket)),
                                                                                    mPageRoot(pageRoot),
                                                                                    mDeadline(mSocket.get_executor().context(),
                                                                                              std::chrono::seconds(60)) {
}

void Session::run() {
    std::cout << "Running session" << std::endl;
    readRequest();
}

void Session::readRequest() {
    checkDeadline();
    auto self = shared_from_this();
    boost::beast::http::async_read(mSocket,
                                   mBuffer,
                                   mRequest,
                                   [self](boost::beast::error_code ec,
                                          std::size_t bytes_transferred){
        boost::ignore_unused(bytes_transferred);
        if(ec){
            self->printErrorCode(ec);
            return;
        }
        self->processRequest();
    });
}

void Session::createResponse() {
    if(mRequest.target().empty() ||
       mRequest.target()[0] != '/' ||
       mRequest.target().find("..") != boost::beast::string_view::npos){
        mResponse.set(boost::beast::http::field::content_type, "text/plain");
        boost::beast::ostream(mResponse.body()) << "Illegal request-target\r\n";
    } else {
        std::string resourceFilePath = *mPageRoot + mRequest.target().to_string();

        if(mRequest.target().back() == '/'){
            resourceFilePath.append("index.html");
        } else {
            resourceFilePath.append(".html");
        }
        std::cout << resourceFilePath << std::endl;
        std::ifstream file;
        file.open(resourceFilePath);
        if(!file.is_open()){
            mResponse.set(boost::beast::http::field::content_type, "text/plain");
            boost::beast::ostream(mResponse.body()) << "404 File not found\r\n";
            return;
        }
        std::stringstream stringstream;
        stringstream << file.rdbuf();
        file.close();

        mResponse.set(boost::beast::http::field::content_type, "text/html");
        boost::beast::ostream(mResponse.body()) << stringstream.str();
    }
}

void Session::processRequest() {
    mResponse.version(mRequest.version());
    mResponse.keep_alive(true);
    switch(mRequest.method()){
        case boost::beast::http::verb::get:
            mResponse.result(boost::beast::http::status::ok);
            mResponse.set(boost::beast::http::field::server, "Beast");
            createResponse();
            break;
        default:
            mResponse.result(boost::beast::http::status::bad_request);
            mResponse.set(boost::beast::http::field::content_type, "text/plain");
            boost::beast::ostream(mResponse.body())
                    << "Invalid request method '"
                       << mRequest.method_string().to_string()
                       << "'";
            break;
    }
    mRequest = {};
    writeResponse();
}

void Session::writeResponse() {
    auto self = shared_from_this();

    mResponse.set(boost::beast::http::field::content_length, mResponse.body().size());

    boost::beast::http::async_write(mSocket,
                                    mResponse,
                                    [self](boost::beast::error_code ec, std::size_t){
                                        self->mResponse = {};
        if(ec){
            self->printErrorCode(ec);
            return;
        }

        self->mDeadline.cancel();
        self->readRequest();
    });
}

void Session::checkDeadline() {
    auto self = shared_from_this();
    mDeadline.async_wait([self](boost::beast::error_code ec){
        if(!ec){
            std::cout << "Timeout" << std::endl;
            self->mSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
        }
    });
}

Session::~Session() {
    boost::system::error_code ec;
    mSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    mSocket.close();
    std::cout << "Session terminated" << std::endl;
}

void Session::printErrorCode(boost::beast::error_code &ec) {
    std::cout << "Error code: "
              << ec.value()
              << " | Message : "
              << ec.message()
              << std::endl;
}
