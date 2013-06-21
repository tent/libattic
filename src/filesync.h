#ifndef FILESYNC_H_
#define FILESYNC_H_
#pragma once

#include <deque>
#include <string>
#include <boost/thread/thread.hpp>

#include "mutexclass.h"
#include "filemanager.h"
#include "accesstoken.h"
#include "filepost.h"

namespace attic { 

class FileSync {
    void Run();
public:
    FileSync(FileManager* fm,
             const AccessToken at,
             const std::string& entity_url,
             const std::string& posts_feed,
             const std::string& post_path);

    ~FileSync();

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
    std::deque<FilePost> post_queue_;

    bool running_;
    boost::thread* thread_;
};

} // namespace
#endif

