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
    int ProcessFilePost(FilePost& p);
    int RaisePullRequest(const FilePost& p, FileInfo& fi);
    bool ExtractFileInfo(FilePost& p, FileInfo& out);

    bool running();
    void set_running(bool r);
    void Run();

    bool ConstructFilepath(const FileInfo& fi, std::string& out);
public:
    FileSync(FileManager* fm,
             const AccessToken at,
             const std::string& entity_url,
             const std::string& posts_feed,
             const std::string& post_path,
             const std::string& master_key);

    ~FileSync();

    void Initialize();
    void Shutdown();

    void PushBack(const FilePost& p);

private:
    AccessToken at_;
    FileManager* file_manager_;
    std::string posts_feed_;
    std::string post_path_;
    std::string entity_url_;
    std::string master_key_;

    MutexClass pq_mtx_;
    std::deque<FilePost> post_queue_;

    MutexClass r_mtx_;
    bool running_;
    boost::thread* thread_;
};

} // namespace
#endif

