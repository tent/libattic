#include "posttree.h"

#include "netlib.h"
#include "filepost.h"
#include "jsonserializable.h"

namespace attic {

PostTree::PostTree(){
    node_count_ = 0;
}

PostTree::~PostTree() {}

void PostTree::PushBackPost(Post* p) {
    if(p) {
        // Check to see if node already exists
        PostNode* node = NULL;
        std::string id = p->version().id();
        if(url_nodes_.find(id) != url_nodes_.end())
            node = url_nodes_[id];
        else
            node = new PostNode();

        //  - set id
        node->post_id = id;
        node->node_id = node_count_;
        node->post = *p;
        node_count_++;

        // Lookup Parents
        //  - set parents
        //  - (parents) add children
        // Pushback to both maps
    }
}

void PostTree::ClearNodes() {
    std::map<std::string, PostNode*>::iterator itr = url_nodes_.begin();
    for(;itr!= url_nodes_.end(); itr++) {
        PostNode* n = itr->second;
        itr->second = NULL;
        if(n) {
            delete n;
            n = NULL;
        }
    }
    url_nodes_.clear();
}

void PostTree::ReturnSerializedTree(std::string& out) {
    Json::Value tree;
    std::map<std::string, PostNode*>::iterator itr = url_nodes_.begin();
    for(;itr!= url_nodes_.end(); itr++) {
        Json::Value node;
        jsn::SerializeObject(&itr->second->post, node);
        tree.append(node);
    }
    jsn::SerializeJsonValue(tree, out);
}

}//namespace

