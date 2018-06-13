//
// Created by Jordan Bare on 6/12/18.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H


#include <thread>
#include "Listener.h"
#include <cereal/access.hpp>
#include <cereal/types/map.hpp>

class Server {
public:
    Server(unsigned short port, unsigned short numThreads, boost::asio::io_context &ioContext, std::string rootDir);
    ~Server();
    void run();
private:
    void displayMenu();
    void createBlog();
    void destroyBlog(std::string blogToDestroy);
    const unsigned short mNumThreads;
    boost::asio::io_context& mIOContext;
    std::shared_ptr<Listener> mListener;
    std::vector<std::thread> mWorkerThreads;
    const std::vector<std::string> mFolderRoots;
    std::mutex mIndexMapMutex;
    std::map<unsigned short,std::string> mIndexMap;
    friend class cereal::access;
    template <class Archive> void serialize(Archive &ar) {
        ar(mIndexMap);
    }

    void writeBlogIndexFile();
};


#endif //SERVER_SERVER_H
