//
// Created by Jordan Bare on 6/4/18.
//

#include <fstream>
#include <cereal/archives/portable_binary.hpp>
#include <boost/asio/bind_executor.hpp>
#include <regex>
#include "Session.h"

Session::Session(boost::asio::ssl::context  &sslContext,
                 boost::asio::ip::tcp::socket socket,
                 std::unique_ptr<CSRFManager> &csrfManager,
                 std::unique_ptr<BlogManager> &blogManager,
                 const std::vector<std::string> &folderRoots): mSocket(std::move(socket)),
                                                               mStream(mSocket, sslContext),
                                                               mStrand(mSocket.get_executor()),
                                                               mDeadline(mSocket.get_executor().context(),std::chrono::seconds(60)),
                                                               mCSRFManager(csrfManager),
                                                               mBlogManager(blogManager),
                                                               mPageRoot(folderRoots[0]),
                                                               mAuthorized(false){
}

void Session::run() {
    std::cout << "1: Session created" << std::endl;
    mStream.async_handshake(boost::asio::ssl::stream_base::server,
                            boost::asio::bind_executor(mStrand,
                                                       std::bind(&Session::onHandshake,
                                                                 shared_from_this(),
                                                                 std::placeholders::_1)));
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
        return close();
    }
    if(ec){
        printErrorCode(ec);
        return;
    }
    processRequest();
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

void Session::createGetResponse() {
    mResponse.result(boost::beast::http::status::ok);
    mResponse.set(boost::beast::http::field::content_type, "text/html");
    std::string resourceFilePath;
    if(forbiddenCheck()){
        resourceFilePath.append(mPageRoot + "403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        if(mBlogManager->checkForBlog(resource)){
        } else {
            resourceFilePath.append(mPageRoot);
            if(resource == "/"){
                resourceFilePath.append("index.html");
            } else if(resource == "/favicon.ico"){
                resourceFilePath.append("favicon.ico");
                mResponse.set(boost::beast::http::field::content_type, "image/vnd.microsoft.icon");
            } else if(resource == "/about"){
                resourceFilePath.append("about.html");
            } else if(resource == "/blogs"){
                resourceFilePath.append("blogs.html");
            } else if(resource == "/login" || "/admin"){
                if(mAuthorized){
                    resourceFilePath.append("admin.html");
                } else {
                    resourceFilePath.append("login.html");
                }
                boost::beast::ostream(mResponse.body()) << insertCSRFToken(resourceFilePath);
                return;
            } else {
                resourceFilePath.append("404.html");
            }
        }
    }
    boost::beast::ostream(mResponse.body()) << readFile(resourceFilePath);
}

void Session::createPostResponse(){
    mResponse.result(boost::beast::http::status::ok);
    mResponse.set(boost::beast::http::field::content_type, "text/html");
    std::string resourceFilePath = mPageRoot;
    if(forbiddenCheck()){
        resourceFilePath.append("403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        std::string body = mRequest.body();
        if(resource == "/checkcreds"){
            unsigned long pwdLoc = body.find("&pwd=");
            std::string usr = body.substr(4, pwdLoc-4);
            unsigned long csrfLoc = body.find("&_csrf=");
            std::string pwd = body.substr(pwdLoc+5, csrfLoc-13);
            if(csrfTokenCheck() && usr == "user" && pwd == "pass"){
                mAuthorized = true;
                resourceFilePath.append("admin.html");
            } else {
                mAuthorized = false;
                resourceFilePath.append("login.html");
            }
            boost::beast::ostream(mResponse.body()) << insertCSRFToken(resourceFilePath);
            return;
        } else if(resource == "/logout"){
            if(csrfTokenCheck()){
                mAuthorized = false;
            }
            resourceFilePath.append("login.html");
        } else if(resource == "/addblog"){

        } else if(resource == "/removeblog"){

        } else {
            resourceFilePath.append("404.html");
        }
    }
    boost::beast::ostream(mResponse.body()) << readFile(resourceFilePath);
}

bool Session::forbiddenCheck() const{
    return mRequest.target().empty() ||
           mRequest.target()[0] != '/' ||
           mRequest.target().find("..") != boost::beast::string_view::npos;
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

void Session::close() {
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

void Session::printErrorCode(boost::beast::error_code &ec) {
    std::cout << "Error code: " << ec.value() << " | Message : " << ec.message() << std::endl;
}

std::string Session::readFile(const std::string &resourceFilePath) const {
    std::ifstream file;
    file.open(resourceFilePath);
    std::stringstream stringstream;
    if(file.is_open()){
        stringstream << file.rdbuf();
        file.close();
    }
    return stringstream.str();
}

std::string Session::insertCSRFToken(const std::string &resourceFilePath) {
    mCSRFToken = (mCSRFManager->addToken());
    std::string page = readFile(resourceFilePath);
    boost::replace_all(page, "CSRF", mCSRFToken);
    return page;
}

bool Session::csrfTokenCheck() {
    std::string requestBody = mRequest.body();
    unsigned long tokenIndex = requestBody.find("_csrf=");
    //always change indexes based on csrf token name
    std::string requestCSRFToken = requestBody.substr(tokenIndex+6, tokenIndex+26);
    if(requestCSRFToken == mCSRFToken){
        mCSRFManager->removeToken(mCSRFToken);
        return true;
    }
    return false;
}

Session::~Session() {
    std::cout << "Session terminated" << std::endl;
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

void Session::sanitizeInput(std::string &input) {

}
