#include "foldercreationlock.h"

namespace attic { 
CreationQueue* CreationQueue::instance_ = 0;
static MutexClass ref_mtx_;
static unsigned int g_ref_count = 0;

CreationQueue* CreationQueue::instance() { 
    ref_mtx_.Lock();
    if(!instance_) { 
        instance_ = new CreationQueue();
    }
    g_ref_count++;
    ref_mtx_.Unlock();
    return instance_;
}
    
void CreationQueue::Shutdown() { 
    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }
}

void CreationQueue::Release() {
    // Perform cleanup
    ref_mtx_.Lock();
    g_ref_count--;
    unsigned hold = g_ref_count;
    ref_mtx_.Unlock();
    if(hold <= 0) {
        if(folder_map_.size() <=0) {
            Shutdown();
        }
    }
}

bool CreationQueue::Lock(const std::string& foldername, const std::string& parent_id) {
    bool ret = false;
    fm_mtx_.Lock();
    FolderMap::iterator itr = folder_map_.find(parent_id);
    if(itr == folder_map_.end()) {
        folder_map_[parent_id][foldername] = true;
        ret = true;
    }
    else {
        if(folder_map_[parent_id].find(foldername) == folder_map_[parent_id].end()) {
            folder_map_[parent_id][foldername] = true;
            ret = true;
        }
    }

    fm_mtx_.Unlock();
    return ret;
}

bool CreationQueue::Unlock(const std::string& foldername, const std::string& parent_id) {
    bool ret = true;
    fm_mtx_.Lock();
    std::map<std::string, bool>::iterator itr = folder_map_[parent_id].find(foldername);
    if(itr != folder_map_[parent_id].end()) {
        folder_map_[parent_id].erase(itr);
    }
    fm_mtx_.Unlock();
    return ret;
}

bool CreationQueue::IsLocked(const std::string& foldername, const std::string& parent_id) {
    bool ret = false;
    fm_mtx_.Lock();
    std::map<std::string, bool>::iterator itr = folder_map_[parent_id].find(foldername);
    if(itr != folder_map_[parent_id].end())
        ret = true;
    fm_mtx_.Unlock();
    return ret;
}


} // namespace
