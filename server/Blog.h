//
// Created by Jordan Bare on 7/19/18.
//

#ifndef SERVER_BLOG_H
#define SERVER_BLOG_H

#include <cereal/access.hpp>
#include <cereal/types/string.hpp>

class Blog {
public:
    Blog(std::string title, std::string dateTime, std::string content, int prevId, int nextId);
    Blog();
private:
    const std::string mTitle;
    const std::string mDateTime;
    const std::string mContent;
    int mPreviousId;
    int mNextId;
    friend class cereal::access;
    template <class Archive> void serialize(Archive &ar) {
        ar(mTitle,mDateTime,mContent,mPreviousId,mNextId);
    }
};


#endif //SERVER_BLOG_H
