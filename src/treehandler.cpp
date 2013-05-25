#include "treehandler.h"

#include "utils.h"

TreeHandler::TreeHandler(const std::string& post_path) {
    post_path_ = post_path;
}

TreeHandler::~TreeHandler() {}

bool TreeHandler::ConstructPostTree(const std::string& post_id, PostTree& out) {
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

    FilePost fp;
    if(RetrievePost(posturl, fp)) {
        if(fp.version()->parents().size()) {
            Version::ParentList::iterator itr = fp.version()->parents().begin();
            for(;itr!= fp.version()->parents().end(); itr++) {
                std::string version_id = itr->version;
                std::string post = itr->post; // if this is not empty then use this id 

            }
        }
    }

    return false;
}

bool TreeHandler::RetrievePost(const std::string& post_url, FilePost& out) {
    Response resp;
    PostHandler<FilePost> ph(access_token_);
    if(ph.Get(post_url, NULL, out, resp) == ret::A_OK)
        return true;
    return false;
}

