#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_
#pragma once

#include <string>

#include "accesstoken.h"
#include "entity.h"
#include "taskfactory.h"
#include "event.h"
#include "task.h"

namespace attic { 

class TentApp;
class FileManager;
class CredentialsManager;
class TaskDelegate;

class TaskManager : public TaskFactoryDelegate, public event::EventListener {
public:
    TaskManager(FileManager* pFm, 
                CredentialsManager* pCm,
                const AccessToken& at,
                const Entity& entity,
                const std::string& tempdir, 
                const std::string& workingdir,
                const std::string& configdir
               );

    ~TaskManager();

    int Initialize();
    int Shutdown();

    virtual void OnEventRaised(const event::Event& event);

    virtual void OnTaskCreate(Task* t);
    virtual void OnTaskInsert(Task* t);

    // Sync Tasks
    int UploadFile(const std::string& filepath, TaskDelegate* pDel);
    int DownloadFile(const std::string& filepath, TaskDelegate* pDel);
    int PollFiles(TaskDelegate* pDel);
    int SyncFiles(TaskDelegate* pDel);
    int SyncFile(const std::string& postid, TaskDelegate* pDel);
    int DeleteFile(const std::string& filepath, TaskDelegate* pDel);
    int RenameFile(const std::string& original_filepath, const std::string& new_filepath);

    int CreateAndSpinOffTask( Task::TaskType tasktype, 
                              const std::string& filepath, 
                              TaskDelegate* pDel);


    // Utility Tasks
    int QueryManifest(void(*callback)(int, char**, int, int));
    int ScanAtticFolder(void(*callback)(int, char**, int, int) = NULL);

    // Info tasks
    int TaskCount(const Task::TaskType);
private:
    TaskFactory             task_factory_; // Local to upload manager

    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;

    AccessToken             access_token_;
    Entity                  entity_;

    std::string             temp_directory_;
    std::string             working_directory_;
    std::string             config_directory_;
};

}//namespace
#endif

