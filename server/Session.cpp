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
                 std::set<std::string> &csrfSet,
                 std::map<unsigned short,std::string> &indexMap,
                 const std::vector<std::string> &folderRoots): mSocket(std::move(socket)),
                                                               mStream(mSocket, sslContext),
                                                               mStrand(mSocket.get_executor()),
                                                               mDeadline(mSocket.get_executor().context(),std::chrono::seconds(10)),
                                                               mCSRFSet(csrfSet),
                                                               mIndexMap(indexMap),
                                                               mFolderRoots(folderRoots),
                                                               mAuthorized(false){
}

void Session::run() {
    std::cout << "Session created" << std::endl;
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
    std::cout << "Handshake successful" << std::endl;
    readRequest();
}

void Session::readRequest() {
    mRequest = {};
    checkDeadline();
    std::cout << "Reading request" << std::endl;
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
        resourceFilePath.append(mFolderRoots[0] + "403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        if(checkForBlogRequest(resource)){
            resourceFilePath.append(mFolderRoots[1]);
        } else {
            resourceFilePath.append(mFolderRoots[0]);
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
    std::string resourceFilePath = mFolderRoots[0];
    if(forbiddenCheck()){
        resourceFilePath.append("403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        if(resource == "/checkcreds"){
            std::string body = mRequest.body();
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
    std::cout << "Sending response" << std::endl;
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

Blog Session::readBlogFromFile(const std::string &resourceFilePath) {
    Blog blog;
    std::ifstream file(resourceFilePath);
    if(file.is_open()){
        cereal::PortableBinaryInputArchive inputArchive(file);
        inputArchive(blog);
        file.close();
    }
    return blog;
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

bool Session::checkForBlogRequest(const std::string &requestString) {
    std::regex regexFormula("^/[1-9][0-9]{0,4}");
    return std::regex_match(requestString, regexFormula);
}

std::string Session::getBlogNumRequested(const std::string &requestString) {
    return requestString.substr(1, requestString.size()-1);
}

std::string Session::insertCSRFToken(std::string &resourceFilePath) {
    std::cout << "Creating token" << std::endl;
    std::random_device rd;
    static thread_local std::mt19937 re{rd()};
    std::uniform_int_distribution<int> urd(97,122);
    do {
        std::stringstream randomStream;
        for(int i=0; i < 20;++i){
            randomStream << char(urd(re));
        }
        mCSRFToken = randomStream.str();
    } while(mCSRFSet.find(mCSRFToken) != mCSRFSet.end());
    mCSRFSet.emplace(mCSRFToken);
    std::string page = readFile(resourceFilePath);
    boost::replace_all(page, "CSRF", mCSRFToken);
    return page;
}

bool Session::csrfTokenCheck() const {
    std::string body = mRequest.body();
    unsigned long tokenIndex = body.find("_csrf=");
    //always change indexes based on csrf token name
    std::string requestCSRFToken = body.substr(tokenIndex+6, tokenIndex+26);
    if(requestCSRFToken == mCSRFToken){
        mCSRFSet.erase(mCSRFToken);
        return true;
    }
    return false;
}

Session::~Session() {
    mCSRFSet.erase(mCSRFToken);
    std::cout << "Session terminated" << std::endl;
}

void Session::checkDeadline() {
    mDeadline.async_wait(std::bind(&Session::onDeadlineCheck,shared_from_this(),std::placeholders::_1));
}

void Session::onDeadlineCheck(boost::system::error_code ec) {
    if(!ec){
        std::cout << "Timer expired" << std::endl;
        close();
    }
}
