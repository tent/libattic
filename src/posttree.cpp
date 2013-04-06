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
        // Extract Value
        // Create PostNode
        //  - set id
        // Lookup Parents
        //  - (parents) add children
        // Pushback to both maps
    }
}

}//namespace

