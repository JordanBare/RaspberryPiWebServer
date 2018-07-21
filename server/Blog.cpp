//
// Created by Jordan Bare on 7/19/18.
//

#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Blog.h"

Blog::Blog(std::string title, std::string dateTime, std::string content, int prevId, int nextId): mTitle(std::move(title)),
                                                                                                  mDateTime(std::move(dateTime)),
                                                                                                  mContent(std::move(content)),
                                                                                                  mPreviousId(prevId),
                                                                                                  mNextId(nextId){}

Blog::Blog() = default;
