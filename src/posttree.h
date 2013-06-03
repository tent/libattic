#ifndef POSTTREE_H_
#define POSTTREE_H_
#pragma once

#include <string>
#include <map>
#include <deque>

#include "post.h"

namespace attic { 

struct PostNode {
    std::string post_id;
    std::deque<PostNode*> parents;
    std::deque<PostNode*> children;
    Post post;
    unsigned int node_id;
};

class PostTree {
public:
    PostTree();
    ~PostTree();

    void ClearNodes();
    void PushBackPost(Post* p, const std::string& raw);
    unsigned int node_count() { return node_count_; }

    void ReturnSerializedTree(std::string& out);
private:
    std::deque<std::string> raw_;
    // Maps contain pointer to the same nodes, each provides a way to look up 
    // values.
    std::map<std::string, PostNode*> url_nodes_; // Post node, key id

    std::string post_url_; // seed post
    unsigned int node_count_;
};

}//namespace
#endif

