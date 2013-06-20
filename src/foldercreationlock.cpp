#include "foldercreationlock.h"

namespace attic { 
CreationQueue* CreationQueue::instance_ = 0;

static MutexClass ref_mtx_;
static unsigned int ref_count_ = 0;

CreationQueue* CreationQueue::instance() { 
    if(!instance_)
        instance_ = new CreationQueue();
    ref_mtx_.Lock();
    ref_count_++;
    std::cout<<"# CURRENT FOLDER LOCK REF COUNT : " << ref_count_ << std::endl;
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
    ref_mtx_.Lock();
    ref_count_--;
    std::cout<<"# CURRENT FOLDER LOCK REF COUNT : " << ref_count_ << std::endl;
    unsigned int hold = ref_count_;
    ref_mtx_.Unlock();
    if(hold <= 0)
        Shutdown();
}

bool CreationQueue::PushBack(const std::string& foldername, const std::string& parent_id) {
    bool ret = false;
    fm_mtx_.Lock();
    std::cout<< "# Locking : " << foldername << " id : " << parent_id << std::endl;
    FolderMap::iterator itr = folder_map_.find(parent_id);
    if(itr == folder_map_.end()) {
        folder_map_[parent_id][foldername] = true;
        ret = true;
    }
    else {
        if(itr->second.find(foldername) == folder_map_[parent_id].end()) {
            folder_map_[parent_id][foldername] = true;
            ret = true;
        }
    }
    fm_mtx_.Unlock();
    return ret;
}

bool CreationQueue::Remove(const std::string& foldername, const std::string& parent_id) {
    bool ret = true;
    fm_mtx_.Lock();
    std::cout<< "# Removing : " << foldername << " id : " << parent_id << std::endl;
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
