#include "posttree.h"

#include "netlib.h"
#include "filepost.h"
#include "jsonserializable.h"

namespace attic {

PostTree::PostTree(){
    node_count_ = 0;
}

PostTree::~PostTree() {}

void PostTree::PushBackPost(Post* p, const std::string& raw) {
    if(p) {
        raw_.push_back(raw);
        std::cout<<" pushing back post ... " << std::endl;
        // Check to see if node already exists
        PostNode* node = NULL;
        std::string id = p->version().id();
        std::cout<<" version id : " << id << std::endl;
        if(url_nodes_.find(id) != url_nodes_.end()) {
            std::cout<<" Duplicate node " << std::endl;
            node = url_nodes_[id];
        }
        else {
            node = new PostNode();
            //  - set id
            node->post_id = id;
            node->node_id = node_count_;
            node->post = *p;
            node_count_++;
            url_nodes_[id] = node;
        }

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
        std::string in;
        jsn::SerializeJsonValue(node, in);
    }
    jsn::SerializeJsonValue(tree, out);
}

}//namespace

