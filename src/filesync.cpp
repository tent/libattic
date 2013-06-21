#include "filesync.h"

namespace attic {

FileSync::FileSync(FileManager* fm,
                   const AccessToken at,
                   const std::string& entity_url,
                   const std::string& posts_feed,
                   const std::string& post_path) {
    running_ = false;
    at_ = at;
    file_manager_ = fm;
    // avoid cow
    posts_feed_.append(posts_feed.c_str(), posts_feed.size());
    post_path_.append(post_path.c_str(), post_path.size());
    entity_url_.append(entity_url.c_str(), entity_url.size());
    utils::FindAndReplace(post_path, "{entity}", entity_url_, post_path_);
}

FileSync::~FileSync() {
    file_manager_ = NULL;
}

void FileSync::Initialize() {
    if(!thread_) {
        running_ = true;
        std::cout<<" starting worker thread ... " << std::endl;
        thread_ = new boost::thread(&FolderSync::Run, this);
    }
}

void FileSync::Shutdown() {
    if(thread_) {
        bool running_ = false;
        std::cout<<" exiting worker thread .. " << std::endl;
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
}

void FileSync::Run() {
    while(running_) {

    }

}

void FileSync::PushBack(const FolderPost& p) {
    pq_mtx_.Lock();
    post_queue_.push_back(p);
    pq_mtx_.Unlock();
}


} //namespace

