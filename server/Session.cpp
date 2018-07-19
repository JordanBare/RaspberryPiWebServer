//
// Created by Jordan Bare on 6/4/18.
//

#include <fstream>
#include <boost/asio/bind_executor.hpp>
#include <regex>
#include "Session.h"

Session::Session(boost::asio::ssl::context  &sslContext,
                 boost::asio::ip::tcp::socket socket,
                 std::unique_ptr<CSRFManager> &csrfManager,
                 std::unique_ptr<BlogManager> &blogManager,
                 std::unique_ptr<CredentialsManager> &credentialsManager,
                 const std::string &pageRootFolder): mSocket(std::move(socket)),
                                                               mStream(mSocket, sslContext),
                                                               mStrand(mSocket.get_executor()),
                                                               mDeadline(mSocket.get_executor().context(),std::chrono::seconds(60)),
                                                               mCSRFManager(csrfManager),
                                                               mBlogManager(blogManager),
                                                               mCredentialsManager(credentialsManager),
                                                               mPageRoot(pageRootFolder),
                                                               mAuthorized(false){
}

Session::~Session() {
    std::cout << "6: Session terminated" << std::endl;
}

void Session::run() {
    std::cout << "1: Session created" << std::endl;
    mStream.async_handshake(boost::asio::ssl::stream_base::server,
                            boost::asio::bind_executor(mStrand,
                                                       std::bind(&Session::onHandshake,
                                                                 shared_from_this(),
                                                                 std::placeholders::_1)));
}

void Session::printErrorCode(boost::beast::error_code &ec) {
    std::cout << "Error code: " << ec.value() << " | Message : " << ec.message() << std::endl;
}

void Session::onHandshake(boost::system::error_code ec) {
    if(ec){
        std::cout << "Handshake error" << std::endl;
        printErrorCode(ec);
        return;
    }
    std::cout << "2: Handshake successful" << std::endl;
    readRequest();
}

void Session::checkDeadline() {
    mDeadline.async_wait(std::bind(&Session::onDeadlineCheck,shared_from_this(),std::placeholders::_1));
}

void Session::onDeadlineCheck(boost::system::error_code ec) {
    if(!ec){
        std::cout << "Timer expired" << std::endl;
        mCSRFManager->removeToken(mCSRFToken);
        close();
    }
}

void Session::readRequest() {
    mRequest = {};
    checkDeadline();
    std::cout << "3: Reading request" << std::endl;
    boost::beast::http::async_read(mStream,
                                   mBuffer,
                                   mRequest,
                                   boost::asio::bind_executor(mStrand,
                                                              std::bind(&Session::handleReadRequest,
                                                                        shared_from_this(),
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)));
}

void Session::handleReadRequest(boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if(ec == boost::beast::http::error::end_of_stream){
        std::cout << "End of Stream" << std::endl;
        close();
        return;
    } if(ec.value() == 335544539){
        std::cout << "Short Read Error" << std::endl;
        mDeadline.cancel();
        close();
        return;
    }
    std::cout << "Successful Read!" << std::endl;
    processRequest();
}

void Session::writeResponse() {
    std::cout << "4: Writing response" << std::endl;
    mResponse.set(boost::beast::http::field::content_length, mResponse.body().size());
    boost::beast::http::async_write(mStream,
                                    mResponse,
                                    boost::asio::bind_executor(mStrand,
                                                               std::bind(&Session::handleWriteResponse,
                                                                         shared_from_this(),
                                                                         std::placeholders::_1,
                                                                         std::placeholders::_2)));
}

void Session::handleWriteResponse(boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    mDeadline.cancel();
    if(ec){
        printErrorCode(ec);
        close();
        return;
    }
    mResponse = {};
    readRequest();
}

void Session::processRequest() {
    mResponse.version(mRequest.version());
    mResponse.keep_alive(true);
    mResponse.set(boost::beast::http::field::server, "Boost Beast");
    switch(mRequest.method()){
        case boost::beast::http::verb::get:
            createGetResponse();
            break;
        case boost::beast::http::verb::post:
            createPostResponse();
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
    writeResponse();
}

bool Session::forbiddenCheck() const {
    return mRequest.target().empty() ||
           mRequest.target()[0] != '/' ||
           mRequest.target().find("..") != boost::beast::string_view::npos;
}

void Session::close() {
    std::cout << "5: Closing" << std::endl;
    mStream.async_shutdown(boost::asio::bind_executor(
            mStrand,
            std::bind(&Session::onShutdown,
                      shared_from_this(),
                      std::placeholders::_1)));
}

void Session::onShutdown(boost::system::error_code ec) {
    if(ec){
        printErrorCode(ec);
    }
    std::cout << "Connection is closed" << std::endl;
}

std::string Session::readFile(const std::string &resourceFilePath) const {
    std::stringstream stringstream;
    std::ifstream file(resourceFilePath);
    if(file.is_open()){
        stringstream << file.rdbuf();
        file.close();
    }
    return stringstream.str();
}

void Session::createGetResponse() {
    mResponse.result(boost::beast::http::status::ok);
    mResponse.set(boost::beast::http::field::content_type, "text/html");
    std::string resourceFilePath;
    if(forbiddenCheck()){
        resourceFilePath.append(mPageRoot + "403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        if(mBlogManager->checkForValidBlogRequest(resource)){
            std::string blog = mBlogManager->retrieveFormattedBlogForRequest(resource);
            if(!blog.empty()){
                mResponse.set(boost::beast::http::field::content_type, "application/json");
                boost::beast::ostream(mResponse.body()) << blog;
                return;
            }
        } else {
            resourceFilePath.append(mPageRoot);
            if(resource == "/"){
                resourceFilePath.append("index.html");
            } else if(resource == "/recentblog"){
                mResponse.set(boost::beast::http::field::content_type, "application/json");
                boost::beast::ostream(mResponse.body()) << mBlogManager->retrieveMostRecentBlog();
                return;
            }
            else if(resource == "/favicon.ico"){
                    resourceFilePath.append("favicon.ico");
                    mResponse.set(boost::beast::http::field::content_type, "image/vnd.microsoft.icon"); }
            else if(resource == "/about"){
                resourceFilePath.append("about.html");
            } else if(resource == "/blogs"){
                resourceFilePath.append("blogs.html");
                mBlogManager->lockRead();
                fillResponseBodyWithFile(resourceFilePath);
                mBlogManager->unlockRead();
                return;
            } else if(resource == "/login" || "/admin"){
                if(mAuthorized){
                    resourceFilePath.append("admin.html");
                } else {
                    resourceFilePath.append("login.html");
                }
                std::string page  = readFile(resourceFilePath);
                mCSRFManager->insertToken(mCSRFToken,page);
                boost::beast::ostream(mResponse.body()) << page;
                return;
            } else {
                resourceFilePath.append("404.html");
            }
        }
    }
    fillResponseBodyWithFile(resourceFilePath);
}

void Session::fillResponseBodyWithFile(const std::string &resourceFilePath) {
    ostream(mResponse.body()) << readFile(resourceFilePath);
}

void Session::createPostResponse() {
    mResponse.result(boost::beast::http::status::ok);
    mResponse.set(boost::beast::http::field::content_type, "text/html");
    std::string resourceFilePath = mPageRoot;
    if (forbiddenCheck()) {
        resourceFilePath.append("403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        std::string body = mRequest.body();
        if (mCSRFManager->compareSessionToken(mCSRFToken, body)) {
            if (mAuthorized) {
                if (resource == "/logout") {
                    mAuthorized = false;
                    resourceFilePath.append("login.html");
                } else if (resource == "/addblog") {
                    mBlogManager->createBlogFromSubmission(body);
                    mBlogManager->writeBlogIndexPage(resourceFilePath);
                    resourceFilePath.append("admin.html");
                } else if (resource == "/removeblog") {
                    mBlogManager->removeBlog(body);
                    mBlogManager->writeBlogIndexPage(resourceFilePath);
                    resourceFilePath.append("admin.html");
                }
            } else {
                    if (resource == "/checkcreds") {
                        if (mCredentialsManager->compareCredentials(body)) {
                            mAuthorized = true;
                            resourceFilePath.append("admin.html");
                        } else {
                            resourceFilePath.append("login.html");
                        }
                    }
            }
            std::string page = readFile(resourceFilePath);
            mCSRFManager->insertToken(mCSRFToken, page);
            boost::beast::ostream(mResponse.body()) << page;
            return;
        } else {
            resourceFilePath.append("404.html");
        }
    }
    fillResponseBodyWithFile(resourceFilePath);
}
