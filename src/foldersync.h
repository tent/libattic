#ifndef FOLDERSYNC_H_
#define FOLDERSYNC_H_
#pragma once


#include <string>
#include <deque>
#include <boost/thread/thread.hpp>

#include "mutexclass.h"
#include "accesstoken.h"
#include "filemanager.h"
#include "folderpost.h"

namespace attic {

class FolderSync {
    void Run();
    bool running();
    void set_running(bool r);
    bool PopFront(FolderPost& out);
    void ValidateFolder(FolderPost& fp);
    bool ValidateFolderTree(const FolderPost& fp);
    bool RetrievePostFromMap(const std::string& post_id, FolderPost& out);
    bool RetrieveFolder(const std::string& post_id, Folder& out);
    bool ConstructFolderpath(const FolderPost& fp, std::string& path_out);
    bool CreateDirectoryTree(const FolderPost& fp);
public:
    FolderSync(FileManager* fm, 
               const AccessToken at,
               const std::string& entity_url,
               const std::string& posts_feed,
               const std::string& post_path);

    ~FolderSync();

    void Initialize();
    void Shutdown();
    
    void PushBack(const FolderPost& p);

private:
    AccessToken at_;
    FileManager* file_manager_;
    std::string posts_feed_;
    std::string post_path_;
    std::string entity_url_;

    /*
    MutexClass pq_mtx_;
    std::deque<FolderPost> post_queue_;
    */

    MutexClass pm_mtx_;
    typedef std::map<std::string, FolderPost> PostMap;
    PostMap post_map_;

    MutexClass r_mtx_;
    bool running_;
    boost::thread* thread_;
};

}//namespace
#endif

