#ifndef CENSUSHANDLER_H_
#define CENSUSHANDLER_H_
#pragma once

#include <string>
#include <deque>
#include <map>

#include "accesstoken.h"
#include "envelope.h"
#include "filepost.h"
#include "folderpost.h"
#include "event.h"

namespace attic { 

class CensusHandler {
    int QueryTimeline(const std::string& post_type,
                      const std::string& fragment, 
                      std::deque<Post>& out);

    void SetReceivedAt(const std::string& post_type,
                       const std::string& fragment, 
                       const std::string& time);

    void DeserializePages(const std::deque<Post>& posts, 
                          std::deque<FilePost>& out);
    void DeserializePages(const std::deque<Post>& posts, 
                          std::deque<FolderPost>& out);
public:
    CensusHandler(const std::string& posts_feed, const AccessToken& at);
    ~CensusHandler();

    bool Inquiry(const std::string& fragment, 
                 std::deque<FilePost>& out);

    bool Inquiry(const std::string& fragment,
                 std::deque<FolderPost>& out);

private:
    // fragment map keeps track of the the since time for each query type
    typedef std::map<std::string, std::string> FragmentMap;
    FragmentMap fragment_map_;

    AccessToken access_token_;
    std::string posts_feed_;

};

} //namespace
#endif

