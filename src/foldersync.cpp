#include "foldersync.h"
#include "sleep.h"

#include "folderhandler.h"

namespace attic {

FolderSync::FolderSync(FileManager* fm, 
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

FolderSync::~FolderSync() {
    file_manager_ = NULL;
}

void FolderSync::Initialize() {
    if(!thread_) {
        running_ = true;
        std::cout<<" starting worker thread ... " << std::endl;
        thread_ = new boost::thread(&FolderSync::Run, this);
    }
}

void FolderSync::Shutdown() {
    if(thread_) {
        bool running_ = false;
        std::cout<<" exiting worker thread .. " << std::endl;
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
}

void FolderSync::Run() {
    std::cout<<" folder sync running .. " << std::endl;
    FolderHandler fh(file_manager_);
    bool val = false;
    while(running_) {
        FolderPost fp;

        pq_mtx_.Lock();
        unsigned int size = post_queue_.size();
        if(size > 0) {
            fp = post_queue_.front();
            post_queue_.pop_front();
            size--;
            val = true;
        }
        pq_mtx_.Unlock();

        if(val) {
            if(fh.ValidateFolderTree(fp.id(), post_path_, at_)) {
                fh.ValidateFolder(fp);
            }
            val = false;
        }
        else {
            sleep::mil(100);
        }
    }
    std::cout<<" Folder Sync finishing..." << std::endl;
}

void FolderSync::PushBack(const FolderPost& p) {
    pq_mtx_.Lock();
    post_queue_.push_back(p);
    pq_mtx_.Unlock();
}


}// namespace

