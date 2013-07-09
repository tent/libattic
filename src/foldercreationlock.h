#ifndef FOLDERCREATIONLOCK_H_
#define FOLDERCREATIONLOCK_H_
#pragma once

#include <map>
#include <string>
#include "mutexclass.h"
#include "sleep.h"

namespace attic { 

class CreationQueue {
protected:
    CreationQueue() {}
    CreationQueue(const CreationQueue& rhs) { }
    CreationQueue operator=(const CreationQueue& rhs) { return *this;}

    ~CreationQueue() {}
private:
    friend class fcl;

    static CreationQueue* instance();
    void Shutdown();
    void Release();
    void Clear();

    bool Lock(const std::string& foldername, const std::string& parent_id);
    bool Unlock(const std::string& foldername, const std::string& parent_id);
    bool IsLocked(const std::string& foldername, const std::string& parent_id);


    // map < parent_id, map <foldername, bool> >
    typedef std::map<std::string, std::map<std::string, bool> > FolderMap;
    MutexClass fm_mtx_;
    FolderMap folder_map_;

    static CreationQueue* instance_;

};

// Folder Creation Lock
// - this is used to prevent duplicate posts when creating posts for directory trees,
//   there can be up to a second delay when creating a folder post and then inserting it 
//   into the local cache
class fcl {
public:
    fcl() {
        cq_ = CreationQueue::instance();
    }

    ~fcl() {
        // unlock any holding locks
        std::map<std::string, std::string>::iterator itr = val_.begin();
        for(;itr!= val_.end(); itr++)
            Unlock(itr->second, itr->first);
        val_.clear();
        cq_->Release();
        cq_ = NULL;
    }

    bool ForceShutdown() {

    }

    bool TryLock(const std::string& foldername, const std::string& parent_id) {
        while(IsLocked(foldername, parent_id))
            sleep::mil(1);
        return Lock(foldername, parent_id);
    }

    bool Lock(const std::string& foldername, const std::string& parent_id) {
        bool ret = cq_->Lock(foldername, parent_id);
        if(ret) val_[parent_id] = foldername;
        return ret;
    }

    bool IsLocked(const std::string& foldername, const std::string& parent_id) {
        return cq_->IsLocked(foldername, parent_id);
    }

    bool Unlock(const std::string& foldername, const std::string& parent_id) {
        bool ret = cq_->Unlock(foldername, parent_id);
        if(ret) val_.erase(parent_id);
        return ret;
    }
private:
    std::map<std::string, std::string> val_;
    CreationQueue* cq_;
};

} // namespace
#endif

