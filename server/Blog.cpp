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

void Blog::modifyBlogPage(std::string &blogPage) {
    std::string stringToFind = "blogentry"; //make sure this matches what's on the page
    size_t replacementPosition = blogPage.find(stringToFind);
    std::string blogString = "<h1>" + mTitle + "</h1><h3>" + mDateTime + "</h3><p>" + mContent + "</p>";
    blogPage.replace(replacementPosition, stringToFind.length(), blogString);
}
