#ifndef POLLING_H_
#define POLLING_H_
#pragma once

#include <boost/timer/timer.hpp>
#include "mutexclass.h"
#include "accesstoken.h"
#include "entity.h"
#include "fileinfo.h"
#include "folderpost.h"

namespace attic {

class FileManager;
class CredentialsManager;
class CensusHandler;
class FileSync;
class FolderSync;

class Polling {
    enum PollState {
        STOP = 0,
        RUNNING,
        PAUSED
    };

    void PollFilePosts();
    void PollDeletedFilePosts();
    void PollSharedFilePosts();
    void PollFolderPosts();
    void PollDeletedFolderPosts();

    void DeleteLocalFile(const FileInfo& fi); // TODO :: temp method, will move to its own job
    void DeleteLocalFolder(const FolderPost& fp);

    bool GetMasterKey(std::string& out);
    bool ValidMasterKey();

    bool running();
    void set_running(bool r);
    void Run();
public:
    Polling(FileManager* fm,
            CredentialsManager* cm,
            const Entity& entity);

    ~Polling();

    void Pause();
    void Resume();

    void Initialize();
    void Shutdown();

    void OnStart();
    void OnFinished();

private:
    boost::timer::cpu_timer timer_;

    Entity                  entity_;
    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;
    // 
    CensusHandler*          census_handler_;
    FolderSync*             folder_sync_;
    FileSync*               file_sync_;

    MutexClass state_mtx_;
    PollState state_;

    MutexClass r_mtx_;
    bool running_;
    boost::thread* thread_;
};


}//namespace

#endif

