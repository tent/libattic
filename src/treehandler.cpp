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
    std::cout<<" original post id : " << post_id << std::endl;
    std::string posturl;
    std::cout<<" finding : " << post_queue_.front().first << std::endl;
    std::cout<<" for : " << post_queue_.front().second << std::endl;
    utils::FindAndReplace(post_queue_.front().first, 
                          "{post}", 
                          post_queue_.front().second, 
                          posturl);
    std::string version;
    while(post_queue_.size()) {
        post_queue_.pop_front();
        FilePost fp;

        bool pass = false;
        if(version.empty()) {
            pass = RetrievePost(posturl, NULL, fp);
        }
        else {
            UrlParams params;
            params.AddValue("version", version);
            pass = RetrievePost(posturl, &params, fp);
        }

        if(pass) {
            // Push into tree
            out.PushBackPost(&fp);
            // extract id's from parents
            if(fp.version().parents().size()) {
                Version::ParentList::const_iterator itr = fp.version().parents().begin();
                for(;itr!= fp.version().parents().end(); itr++) {
                    PostPair parent_p;
                    if(version == itr->version)
                        break;
                    version = itr->version;
                    if(!itr->post.empty())
                        parent_p.first = itr->post;
                    else
                        parent_p.first = post_path_;
                    post_queue_.push_back(parent_p);
                }
            }
        }
    }
    
    return true;
}

bool TreeHandler::RetrievePost(const std::string& post_url, UrlParams* params, FilePost& out) {
    Response resp;
    PostHandler<FilePost> ph(access_token_);
    if(ph.Get(post_url, params, out, resp) == ret::A_OK) {
        std::cout<<" TREE RESPONSE : " << resp.body << std::endl;
        return true;
    }
    return false;
}

} //namespace
