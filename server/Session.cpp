//
// Created by Jordan Bare on 6/4/18.
//

#include <fstream>
#include <cereal/archives/portable_binary.hpp>
#include <boost/asio/bind_executor.hpp>
#include <regex>
#include "Session.h"

Session::Session(boost::asio::ssl::context& sslContext,
                 boost::asio::ip::tcp::socket socket,
                 std::map<unsigned short,std::string> &indexMap,
                 const std::vector<std::string> &folderRoots): mSocket(std::move(socket)),
                                                               mStream(mSocket, sslContext),
                                                               mStrand(mSocket.get_executor()),
                                                               mIndexMap(indexMap),
                                                               mFolderRoots(folderRoots),
                                                               mAuthorized(false){
}

void Session::run() {
    std::cout << "Running session" << std::endl;
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
        return printErrorCode(ec);
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
        if(resource == "/"){
            resourceFilePath.append(mFolderRoots[0] + "index.html");
        } else if(resource == "/favicon.ico"){
            resourceFilePath.append(mFolderRoots[0] + "favicon.ico");
            mResponse.set(boost::beast::http::field::content_type, "image/vnd.microsoft.icon");
        } else if(resource == "/about"){
            resourceFilePath.append(mFolderRoots[0] + "about.html");
        } else if(resource == "/blogs"){
            resourceFilePath.append(mFolderRoots[0] + "blogs.html");
        } else if(resource == "/login"){
            resourceFilePath.append(mFolderRoots[0] + "login.html");
        } else if(resource == "/admin" && mAuthorized){
            resourceFilePath.append(mFolderRoots[0] + "admin.html");
        } else if(checkForRequestedBlog(resource)){
            resourceFilePath.append(mFolderRoots[1] + getBlogNumRequested(resource) + ".txt");
            Blog blog = readBlogFromFile(resourceFilePath);
            boost::beast::ostream(mResponse.body()) << blog.getBlogPage();
            return;
        } else {
            resourceFilePath.append(mFolderRoots[0] + "404.html");
        }
    }
    boost::beast::ostream(mResponse.body()) << readFile(resourceFilePath);
}

void Session::createPostResponse(){
    mResponse.result(boost::beast::http::status::ok);
    mResponse.set(boost::beast::http::field::content_type, "text/html");
    std::string resourceFilePath;
    if(forbiddenCheck()){
        resourceFilePath.append(mFolderRoots[0] + "403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        if(resource == "/checkcreds"){
            std::string body = mRequest.body();
            unsigned long middle = body.find("&pwd");
            std::string usr = body.substr(4, middle-4);
            std::string pwd = body.substr(middle+5, body.size()-1);
            std::cout << usr << " " << pwd << std::endl;
            if(usr == "user" && pwd == "pass"){
                mAuthorized = true;
                resourceFilePath.append(mFolderRoots[0] + "admin.html");
            } else {
                mAuthorized = false;
                resourceFilePath.append(mFolderRoots[0] + "login.html");
            }
        } else {
            resourceFilePath.append(mFolderRoots[0] + "404.html");
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
    std::cout << "Error code: "
              << ec.value()
              << " | Message : "
              << ec.message()
              << std::endl;
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

bool Session::checkForRequestedBlog(const std::string &requestString) {
    std::regex regexFormula("^/blog[1-9][0-9]{0,3}");
    return std::regex_match(requestString, regexFormula) && mIndexMap.count(boost::lexical_cast<unsigned short>(getBlogNumRequested(requestString)));
}

std::string Session::getBlogNumRequested(const std::string &requestString) {
    return requestString.substr(5, requestString.size()-1);
}
