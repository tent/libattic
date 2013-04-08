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
        if(url_nodes_.find(p->id()) != url_nodes_.end())
            node = url_nodes_[p->id()];
        else
            node = new PostNode();

        //  - set id
        node->post_id = p->id();
        node->node_id = node_count_;
        node_count_++;

        // Lookup Parents
        //  - set parents
        //  - (parents) add children
        // Pushback to both maps
    }
}

}//namespace

