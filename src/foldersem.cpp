#include "foldersem.h"
#include "sleep.h"

namespace attic { 

FolderMap* FolderMap::instance_ = 0;
static MutexClass ref_mtx_;
static unsigned int g_ref_count = 0;

FolderMap::FolderMap() {

}

FolderMap* FolderMap::instance() {
    if(!instance_)
        instance_ = new FolderMap();
    ref_mtx_.Lock();
    g_ref_count++;
    ref_mtx_.Unlock();
    return instance_;
}

void FolderMap::Shutdown() {
    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }
}

void FolderMap::Release() {
    ref_mtx_.Lock();
    g_ref_count--;
    unsigned int hold = g_ref_count;
    ref_mtx_.Unlock();
    if(hold <= 0 )
        Shutdown();
}

bool FolderMap::AquireRead(const std::string& folder_id) {
    bool ret = false;
    std::cout<<" AQUIRE READ BEGIN " << folder_id << std::endl;
    while(!ret) {
        req_mtx_.Lock();
        bool write_in_progress = false;
        if(requested_write_map_.find(folder_id) != requested_write_map_.end())
            write_in_progress = requested_write_map_[folder_id];
        req_mtx_.Unlock();

        if(!write_in_progress) {
            rm_mtx_.Lock();
            read_map_[folder_id] += 1;
            rm_mtx_.Unlock();
            ret = true;
            break;
        }
        sleep::mil(10);
    }
    std::cout<<" AQUIRE READ END " << folder_id << std::endl;
    return ret;
}

bool FolderMap::AquireWrite(const std::string& folder_id) {
    bool ret = false;
    std::cout<<" AQUIRE WRITE BEGIN " << folder_id << std::endl;
    while(!ret) {
        rm_mtx_.Lock();
        unsigned int read_count = read_map_[folder_id];
        rm_mtx_.Unlock();

        if(read_count > 0) {
            req_mtx_.Lock();
            requested_write_map_[folder_id] = true;
            req_mtx_.Unlock();
        }

        if(read_count <=0) {
            wm_mtx_.Lock();
            if(write_map_.find(folder_id) == write_map_.end()) {
                write_map_[folder_id] = true;

                req_mtx_.Lock();
                requested_write_map_.erase(folder_id);
                req_mtx_.Unlock();
                ret = true;
            }
            wm_mtx_.Unlock();
            if(ret) break;
        }
        sleep::mil(10);
    }
    std::cout<<" AQUIRE WRITE END " << folder_id << std::endl;
    return ret;
}

bool FolderMap::ReleaseRead(const std::string& folder_id) {
    bool ret = false;
    std::cout << " RELEASE READ " << std::endl;
    rm_mtx_.Lock();
    read_map_[folder_id]--;
    rm_mtx_.Unlock();
    return ret;
}

bool FolderMap::ReleaseWrite(const std::string& folder_id) {
    bool ret = false;

    wm_mtx_.Lock();
    write_map_.erase(folder_id);
    wm_mtx_.Unlock();

    std::cout << " RELEASE WRITE " << std::endl;
    return ret;
}

FolderSem::FolderSem() {
    fm_ = FolderMap::instance();
}

FolderSem::~FolderSem() {
    if(!write_.empty())
        fm_->ReleaseWrite(write_);

    if(reads_.size()) {
        std::map<std::string, bool>::iterator itr = reads_.begin();
        for(;itr!= reads_.end();itr++) {
            fm_->ReleaseRead(itr->first);
        }
    }
    fm_->Release();
    fm_ = NULL;
}

bool FolderSem::AquireRead(const std::string& folder_id) {
    return fm_->AquireRead(folder_id);
}

bool FolderSem::AquireWrite(const std::string& folder_id) {
    return fm_->AquireWrite(folder_id);
}

bool FolderSem::ReleaseRead(const std::string& folder_id) {
    return fm_->ReleaseRead(folder_id);
}

bool FolderSem::ReleaseWrite(const std::string& folder_id) {
    return fm_->ReleaseWrite(folder_id);
}


} //namespace

