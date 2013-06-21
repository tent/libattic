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

    MutexClass pq_mtx_;
    std::deque<FolderPost> post_queue_;

    bool running_;
    boost::thread* thread_;
};

}//namespace
#endif

