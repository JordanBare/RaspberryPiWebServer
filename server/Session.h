//
// Created by Jordan Bare on 6/4/18.
//

#ifndef SERVER_HTTPWORKER_H
#define SERVER_HTTPWORKER_H

#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket, std::shared_ptr<std::string> &pageRoot);
    ~Session();
    void run();
    void readRequest();
    void processRequest();
    void createResponse();
    void writeResponse();
    void checkDeadline();
private:
    void printErrorCode(boost::beast::error_code &ec);
    boost::asio::ip::tcp::socket mSocket;
    std::shared_ptr<std::string> mPageRoot;
    boost::beast::flat_buffer mBuffer{8192};
    boost::beast::http::request<boost::beast::http::dynamic_body> mRequest;
    boost::beast::http::response<boost::beast::http::dynamic_body> mResponse;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> mDeadline;
};


#endif //SERVER_HTTPWORKER_H
