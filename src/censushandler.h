#ifndef CENSUSHANDLER_H_
#define CENSUSHANDLER_H_
#pragma once

#include <string>
#include <deque>
#include <map>


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
    // fragment map keeps track of the the since time for each query type
    typedef std::map<std::string, std::string> FragmentMap;
    FragmentMap fragment_map_;

    AccessToken access_token_;
    std::string posts_feed_;

};

} //namespace
#endif

