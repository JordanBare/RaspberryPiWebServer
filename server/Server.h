//
// Created by Jordan Bare on 6/12/18.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H


#include <thread>
#include "Listener.h"
#include <cereal/access.hpp>
#include <cereal/types/map.hpp>
#include <boost/asio/ssl/context.hpp>

class Server {
public:
    Server(unsigned short port, unsigned short numThreads, std::string rootDir);
    ~Server();
    void run(unsigned short numThreads);
private:
    void displayMenu();
    /*
    void createBlogFiles();
    void destroyBlog(std::string blogToDestroy);
    void writeBlogIndexFile();
    void readBlogIndexFile();
    void writeBlogListPageFile();
    Blog createBlogFromInfo() const;
     */
    void loadCertificate();
    std::string get_Password();

    boost::asio::io_context mIOContext;
    boost::asio::ssl::context mSSLContext;
    std::string mRootDir;
    std::shared_ptr<Listener> mListener;
    std::vector<std::thread> mWorkerThreads;
    std::map<unsigned short,std::string> mIndexMap;
    friend class cereal::access;
    template <class Archive> void serialize(Archive &ar) {
        ar(mIndexMap);
    }
};


#endif //SERVER_SERVER_H
