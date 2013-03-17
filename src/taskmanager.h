#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_
#pragma once

#include <string>

#include "accesstoken.h"
#include "entity.h"
#include "taskfactory.h"
#include "event.h"
#include "task.h"

class TentApp;
class FileManager;
class CredentialsManager;
class TaskDelegate;

class TaskManager : public TaskFactoryDelegate, public event::EventListener {
public:
    TaskManager(TentApp* pApp, 
                FileManager* pFm, 
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

    int CreateAndSpinOffTask( Task::TaskType tasktype, 
                              const std::string& filepath, 
                              TaskDelegate* pDel);


    // Utility Tasks
    int QueryManifest(void(*callback)(int, char**, int, int));

    // Info tasks
    int TaskCount(const Task::TaskType);
private:
    TaskFactory             m_TaskFactory; // Local to upload manager

    TentApp*                m_pApp;
    FileManager*            m_pFileManager;
    CredentialsManager*     m_pCredentialsManager;

    AccessToken             m_AccessToken;
    Entity                  m_Entity;

    std::string             m_TempDir;
    std::string             m_WorkingDir;
    std::string             m_ConfigDir;
};

#endif

