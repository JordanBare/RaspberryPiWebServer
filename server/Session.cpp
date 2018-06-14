//
// Created by Jordan Bare on 6/4/18.
//

#include <fstream>
#include <cereal/archives/portable_binary.hpp>
#include <regex>
#include "Session.h"

Session::Session(boost::asio::ip::tcp::socket socket,
                 std::map<unsigned short,std::string> &indexMap,
                 const std::vector<std::string> &folderRoots): mSocket(std::move(socket)),
                                                               mDeadline(mSocket.get_executor().context(),
                                                                         std::chrono::seconds(60)),
                                                               mIndexMap(indexMap),
                                                               mFolderRoots(folderRoots){
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
    mResponse.set(boost::beast::http::field::content_type, "text/html");
    std::string resourceFilePath;
    if(mRequest.target().empty() ||
       mRequest.target()[0] != '/' ||
       mRequest.target().find("..") != boost::beast::string_view::npos){
        resourceFilePath.append(mFolderRoots[0] + "403.html");
    } else {
        std::string resource = mRequest.target().to_string();
        if(resource == "/"){
            resourceFilePath.append(mFolderRoots[0] + "index.html");
        } else if(resource == "/about") {
            resourceFilePath.append(mFolderRoots[0] + "about.html");
        } else if(resource == "/bloglist") {
            resourceFilePath.append(mFolderRoots[0] + "bloglist.html");
        } else if(checkForRequestedBlog(resource)) {

            resourceFilePath.append(mFolderRoots[1] + getBlogNumRequested(resource) + ".txt");
            Blog blog = readBlogFromFile(resourceFilePath);

            resourceFilePath = mFolderRoots[0] + "blogtemplate.html";
            std::string blogString = readFile(resourceFilePath);
            blog.modifyBlogPage(blogString);

            boost::beast::ostream(mResponse.body()) << blogString;
            return;
        } else {
            resourceFilePath.append(mFolderRoots[0] + "404.html");
        }
    }
    boost::beast::ostream(mResponse.body()) << readFile(resourceFilePath);
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

void Session::processRequest() {
    mResponse.version(mRequest.version());
    mResponse.keep_alive(true);
    switch(mRequest.method()){
        case boost::beast::http::verb::get:
            mResponse.result(boost::beast::http::status::ok);
            mResponse.set(boost::beast::http::field::server, "Boost Beast");
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
