#ifndef FOLDERSEM_H_
#define FOLDERSEM_H_
#pragma once

#include <map>
#include <string>
#include "mutexclass.h"

namespace attic {

class FolderMap {
    friend class FolderSem;
    FolderMap();
    FolderMap(const FolderMap& rhs) {}
    FolderMap operator=(const FolderMap& rhs) { return *this; }
    ~FolderMap() {}

    static FolderMap* instance();

    void Shutdown();
    void Release();

    bool AquireRead(const std::string& folder_id);
    bool AquireWrite(const std::string& folder_id);
    bool ReleaseRead(const std::string& folder_id);
    bool ReleaseWrite(const std::string& folder_id);

    MutexClass rm_mtx_;
    std::map<std::string, unsigned int> read_map_;
    MutexClass wm_mtx_;
    std::map<std::string, bool> write_map_;

    MutexClass req_mtx_;
    std::map<std::string, bool> requested_write_map_;

    static FolderMap* instance_;
};

class FolderSem {
public:
    FolderSem();
    ~FolderSem();

    // multiple reads, single writes
    bool AquireRead(const std::string& folder_id); // using a directory
    bool AquireWrite(const std::string& folder_id); // renaming a directory
    bool ReleaseRead(const std::string& folder_id);
    bool ReleaseWrite(const std::string& folder_id);
private:
    FolderMap* fm_;
    std::map<std::string, bool> reads_;
    std::string write_;
};


}//namespace
#endif

