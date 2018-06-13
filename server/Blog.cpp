//
// Created by Jordan Bare on 6/11/18.
//

#include "Blog.h"
#include <utility>

Blog::Blog(std::string title, std::string content): mTitle(std::move(title)),
                                                    mContent(std::move(content)){
    std::stringstream stream;
    stream << boost::posix_time::second_clock::local_time();
    mDateTime = stream.str();
}

std::string Blog::getTitle(){
    return mTitle;
}

std::string Blog::getContent() {
    return mContent;
}

std::string Blog::getDateTime() {
    return mDateTime;
}
