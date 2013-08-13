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
#include "sharedfilepost.h"

namespace attic { 

class FileSync {
    void ProcessFiles();
    void ProcessSharedFiles();

    int ProcessFilePost(FilePost& p);
    int ProcessSharedFilePost(SharedFilePost& p);


    int RaisePullRequest(const FilePost& p, FileInfo& fi);
    bool ExtractFileInfo(FilePost& p, FileInfo& out);
    bool ExtractFileInfo(SharedFilePost& p, FileInfo& out);

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
    void PushBack(const SharedFilePost& p);

private:
    AccessToken at_;
    FileManager* file_manager_;
    std::string posts_feed_;
    std::string post_path_;
    std::string entity_url_;
    std::string master_key_;

    MutexClass pq_mtx_;
    std::deque<FilePost> post_queue_;

    MutexClass spq_mtx_;
    std::deque<SharedFilePost> shared_post_queue_;

    MutexClass r_mtx_;
    bool running_;
    boost::thread* thread_;
};

} // namespace
#endif

