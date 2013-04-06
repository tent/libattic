#ifndef POSTTREE_H_
#define POSTTREE_H_
#pragma once

#include <string>
#include <map>
#include <deque>

#include <boost/graph/adjacency_list.hpp>

#include "post.h"

namespace attic { 

struct PostNode {
    std::string post_url;
    std::deque<PostNode*> parents;
    std::deque<PostNode*> children;
    unsigned int node_id;
};

class PostTree {
public:
    PostTree();
    ~PostTree();

    void PushBackPost(Post* p);
private:
    boost::adjacency_list<boost::listS, boost::vecS, boost::directedS> graph_;
    // Maps contain pointer to the same nodes, each provides a way to look up 
    // values.
    std::map<std::string, PostNode*> url_nodes_; // Post node, key url
    std::map<unsigned int, PostNode*> id_nodes_; // Post node, key id

    std::string post_url_; // seed post
    unsigned int node_count_;
};

}//namespace
#endif

