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

bool TreeHandler::ConstructPostTree(const std::string& post_path, 
                                    const std::string& post_id, 
                                    PostTree& out) {
    typedef std::pair<std::string, std::string> PostPair; // path, version
    std::deque<PostPair> post_queue;

    std::cout<<" original post id : " << post_id << std::endl;
    std::string posturl;
    utils::FindAndReplace(post_path, 
                          "{post}", 
                          post_id, 
                          posturl);

    PostPair one;
    one.first = posturl;
    one.second = "";
    post_queue.push_back(one);

    // input id (if version get version else get latest)
    // retrieve post
    //  look at parents version
    //  set parent version

    while(post_queue.size()) {
        std::string url = post_queue.front().first;
        std::string version = post_queue.front().second;
        post_queue.pop_front();

        FilePost fp;
        bool pass = false;
        std::string raw;
        if(version.empty()) {
            pass = RetrievePost(url, NULL, fp, raw);
        }
        else {
            UrlParams params;
            params.AddValue("version", version);
            pass = RetrievePost(url, &params, fp, raw);
        }

        if(pass) {
            // Push back last post
            out.PushBackPost(&fp, raw);
            // extract id's from parents
            if(fp.version().parents().size()) {
                std::cout<<" PARENT SIZE : " << fp.version().parents().size() << std::endl;
                Version::ParentList::const_iterator itr = fp.version().parents().begin();
                for(;itr!= fp.version().parents().end(); itr++) {
                    PostPair parent_p;
                    parent_p.first = posturl; // TODO if parent post is a different url or id, handle
                    parent_p.second = itr->version;
                    post_queue.push_back(parent_p);
                }
            }
        }
    }
    
    return true;
}

bool TreeHandler::RetrievePost(const std::string& post_url, 
                               UrlParams* params, 
                               FilePost& out,
                               std::string& raw) {
    Response resp;
    PostHandler<FilePost> ph(access_token_);
    if(ph.Get(post_url, params, out, resp) == ret::A_OK) {
        std::cout<<" TREE RESPONSE : " << resp.body << std::endl;
        raw = resp.body;
        return true;
    }
    return false;
}

} //namespace
