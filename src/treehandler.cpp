#include "treehandler.h"

#include "utils.h"
#include "posthandler.h"

namespace attic { 

TreeHandler::TreeHandler(const AccessToken& at, 
                         const std::string& post_path) {
    access_token_ = at;
    post_path_ = post_path;
}

TreeHandler::~TreeHandler() {}

bool TreeHandler::ConstructPostTree(const std::string& post_id, PostTree& out) {
    // first - path, second - id
    PostPair post_pair(post_path_, post_id);
    post_queue_.push_back(post_pair);

    while(post_queue_.size()) {
        std::string posturl;
        utils::FindAndReplace(post_queue_.front().first, 
                              "{post}", 
                              post_queue_.front().second, 
                              posturl);
        post_queue_.pop_front();
        FilePost fp;
        if(RetrievePost(posturl, fp)) {
            // Push into tree
            out.PushBackPost(&fp);
            // extract id's from parents
            if(fp.version().parents().size()) {
                Version::ParentList::const_iterator itr = fp.version().parents().begin();
                for(;itr!= fp.version().parents().end(); itr++) {
                    PostPair parent_p;
                    parent_p.second = itr->version;
                    if(!itr->post.empty())
                        parent_p.first = itr->post;
                    else
                        parent_p.first = post_path_;
                    post_queue_.push_back(parent_p);
                }
            }
        }
        else {
            return false;
        }
    }
    
    return true;
}

bool TreeHandler::RetrievePost(const std::string& post_url, FilePost& out) {
    Response resp;
    PostHandler<FilePost> ph(access_token_);
    if(ph.Get(post_url, NULL, out, resp) == ret::A_OK)
        return true;
    return false;
}

} //namespace
