#ifndef CENSUSHANDLER_H_
#define CENSUSHANDLER_H_
#pragma once

#include <string>
#include <deque>


#include "accesstoken.h"
#include "filepost.h"
#include "event.h"

namespace attic { 

class CensusHandler {
    int QueryTimeline(const std::string& fragment, std::deque<FilePost>& out);
public:
    CensusHandler(const std::string& posts_feed, const AccessToken& at);
    ~CensusHandler();

    bool Inquiry(const std::string& fragment, std::deque<FilePost>& out);

private:
    AccessToken access_token_;
    std::string posts_feed_;

    std::string since_time_;
};

} //namespace
#endif

