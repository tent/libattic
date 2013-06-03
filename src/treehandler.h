#ifndef TREEHANDLER_H_
#define TREEHANDLER_H_
#pragma once

#include <deque>
#include <string>
#include "posttree.h"
#include "filepost.h"
#include "accesstoken.h"
#include "urlparams.h"

namespace attic { 

class TreeHandler {
    bool RetrievePost(const std::string& post_url, 
                      UrlParams* params, 
                      FilePost& out,
                      std::string& raw);
public:
    TreeHandler(const AccessToken& at, 
                const std::string& post_path);
    ~TreeHandler();

    bool ConstructPostTree(const std::string& post_id, PostTree& out);
private:
    typedef std::pair<std::string, std::string> PostPair; // path, id
    std::deque<PostPair> post_queue_;
    std::string post_path_;
    AccessToken access_token_;
};

} //namespace
#endif

