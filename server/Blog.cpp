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

std::string Blog::getTitle() const {
    return mTitle;
}

std::string Blog::getBlogPage() {
    std::string blogString = "<article class=\"contentColor fontColor\" style=\"padding:14px\"><h1>"
                             + mTitle + "</h1><h3>" + mDateTime + "</h3><p>" + mContent + "</p></article>";
    return blogString;
}
