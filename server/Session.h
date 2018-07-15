//
// Created by Jordan Bare on 6/4/18.
//

#ifndef SERVER_HTTPWORKER_H
#define SERVER_HTTPWORKER_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include "Blog.h"
#include "CSRFManager.h"
#include "BlogManager.h"
#include "CredentialsManager.h"

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ssl::context& sslContext, boost::asio::ip::tcp::socket socket, std::unique_ptr<CSRFManager> &csrfManager, std::unique_ptr<BlogManager> &blogManager, std::unique_ptr<CredentialsManager> &credentialsManager, const std::vector<std::string> &folderRoots);
    ~Session();
    void run();
private:
    void onHandshake(boost::system::error_code ec);
    void readRequest();
    void handleReadRequest(boost::system::error_code ec, std::size_t bytes_transferred);
    void processRequest();
    void createGetResponse();
    void createPostResponse();
    bool forbiddenCheck() const;
    void writeResponse();
    void handleWriteResponse(boost::system::error_code ec, std::size_t bytes_transferred);
    void close();
    void onShutdown(boost::system::error_code ec);
    void printErrorCode(boost::beast::error_code &ec);
    std::string readFile(const std::string &resourceFilePath) const;
    void checkDeadline();
    void onDeadlineCheck(boost::system::error_code ec);

    boost::asio::ip::tcp::socket mSocket;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> mStream;
    boost::asio::strand<boost::asio::io_context::executor_type> mStrand;
    const std::string mPageRoot;
    bool mAuthorized;
    std::unique_ptr<CSRFManager> &mCSRFManager;
    std::unique_ptr<BlogManager> &mBlogManager;
    std::unique_ptr<CredentialsManager> &mCredentialsManager;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> mDeadline;
    std::string mCSRFToken;
    //previous flat is 8192
    boost::beast::flat_buffer mBuffer{8192};
    boost::beast::http::request<boost::beast::http::string_body> mRequest;
    boost::beast::http::response<boost::beast::http::dynamic_body> mResponse;
};

#endif //SERVER_HTTPWORKER_H
