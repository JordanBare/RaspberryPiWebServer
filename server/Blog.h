//
// Created by Jordan Bare on 6/11/18.
//

#ifndef SERVER_BLOG_H
#define SERVER_BLOG_H


#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class Blog {
public:
    Blog() = default;
    Blog(std::string title, std::string content);
    std::string getTitle() const;
    std::string getBlogPage();
private:
    friend class cereal::access;
    template <class Archive> void serialize(Archive &ar) {
        ar(mTitle, mContent, mDateTime);
    }
    std::string mTitle;
    std::string mContent;
    std::string mDateTime;
};


#endif //SERVER_BLOG_H
