//
// Created by Jordan Bare on 6/4/18.
//

#ifndef SERVER_HTTPWORKER_H
#define SERVER_HTTPWORKER_H

#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include "Blog.h"

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket, std::map<unsigned short,std::string> &indexMap, const std::vector<std::string> &folderRoots);
    ~Session();
    void run();
    void readRequest();
    void processRequest();
    void createResponse();
    void writeResponse();
    void checkDeadline();
    std::string readFile(const std::string &resourceFilePath) const;
private:
    void printErrorCode(boost::beast::error_code &ec);
    Blog readBlogFromFile(const std::string &resourceFilePath);
    bool checkForRequestedBlog(const std::string &requestString);
    std::string getBlogNumRequested(const std::string &requestString);
    boost::asio::ip::tcp::socket mSocket;
    //previous flat is 8192
    boost::beast::flat_buffer mBuffer{1000};
    boost::beast::http::request<boost::beast::http::dynamic_body> mRequest;
    boost::beast::http::response<boost::beast::http::dynamic_body> mResponse;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> mDeadline;
    std::map<unsigned short, std::string> &mIndexMap;
    const std::vector<std::string> &mFolderRoots;


};


#endif //SERVER_HTTPWORKER_H
