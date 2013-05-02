#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_
#pragma once

#include <string>

#include "accesstoken.h"
#include "entity.h"
#include "taskfactory.h"
#include "event.h"
#include "task.h"
#include "mutexclass.h"

namespace attic { 

class TentApp;
class FileManager;
class CredentialsManager;
class TaskDelegate;

class TaskManager : public event::EventListener {
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

    void Dispatch();

    int Initialize();
    int Shutdown();

    virtual void OnEventRaised(const event::Event& event);

    virtual void OnTaskCreate(Task* t);
    virtual void OnTaskInsert(Task* t);

    Task* GetTentTask(const TaskContext& tc);
    // Sync Tasks
    void UploadFile(const std::string& filepath, TaskDelegate* pDel);
    void DownloadFile(const std::string& filepath, TaskDelegate* pDel);
    void PollFiles(TaskDelegate* pDel);
    void SyncFiles(TaskDelegate* pDel);
    void SyncFile(const std::string& postid, TaskDelegate* pDel);
    void DeleteFile(const std::string& filepath, TaskDelegate* pDel);
    void RenameFile(const std::string& original_filepath, const std::string& new_filename);
    void RenameFolder(const std::string& original_folderpath, const std::string& new_foldername);

    // Utility Tasks
    int QueryManifest(void(*callback)(int, char**, int, int));
    int ScanAtticFolder(void(*callback)(int, char**, int, int) = NULL);

    // Info tasks
    int TaskCount(const Task::TaskType);

    void PushContextBack(TaskContext& tc);
    void RetrieveContextQueue(TaskContext::ContextQueue& out);

    FileManager* file_manager() { return file_manager_; }
    CredentialsManager* credentials_manager() { return credentials_manager_; }
    const AccessToken& access_token() const { return access_token_; }
    const Entity& entity() const { return entity_; }
    const std::string temp_directory() const { return temp_directory_; }
    const std::string working_directory() const { return working_directory_; }
    const std::string config_directory() const { return config_directory_; }
private:
    TaskFactory             task_factory_; // Local to upload manager

    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;

    AccessToken             access_token_;
    Entity                  entity_;

    std::string             temp_directory_;
    std::string             working_directory_;
    std::string             config_directory_;

    MutexClass cxt_mtx;
    TaskContext::ContextQueue context_queue_; // hold queue
};

}//namespace
#endif

