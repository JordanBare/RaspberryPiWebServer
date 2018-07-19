//
// Created by Jordan Bare on 7/19/18.
//

#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Blog.h"

Blog::Blog(std::string title, std::string content, std::string dateTime, int prevId, int nextId): mTitle(std::move(title)), mContent(std::move(content)), mDateTime(std::move(dateTime)), mPreviousId(prevId), mNextId(nextId){
    /*
    std::stringstream stream;
    stream << boost::posix_time::second_clock::local_time();
    mDateTime = stream.str();
     */
}

Blog::Blog() {}
