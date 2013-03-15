#ifndef TASKFACTORY_H_
#define TASKFACTORY_H_
#pragma once

#include <string>
#include <deque>

#include "accesstoken.h"
#include "mutexclass.h"
#include "entity.h"
#include "task.h"
#include "taskpool.h"

class TentApp;
class FileManager;
class ConnectionManager;
class CredentialsManager;
class Credentials;
class TaskArbiter;
class TaskDelegate;

class TaskFactoryDelegate {
public:
    TaskFactoryDelegate() {}
    ~TaskFactoryDelegate() {}

    virtual void OnTaskCreate(Task* t) = 0;
    virtual void OnTaskInsert(Task* t) = 0;
};

class TaskFactory : public MutexClass {
private:
    void TaskFinished(int code, Task* pTask);

    Task* CreateNewTentTask( Task::TaskType type,                  
                             TentApp* pApp,                  
                             FileManager* pFm,               
                             CredentialsManager* pCm,        
                             TaskArbiter* pTa,
                             TaskFactory* pTf,
                             const AccessToken& at,          
                             const Entity& entity,      
                             const std::string& filepath,    
                             const std::string& tempdir,     
                             const std::string& workingdir,  
                             const std::string& configdir,   
                             TaskDelegate* callbackDelegate);

    Task* CreateNewManifestTask( Task::TaskType type,
                                 FileManager* pFm,
                                 void (*callback)(int, char**, int, int));

    void PushBackTask(Task* t, TaskFactoryDelegate* delegate);
    void LogUnknownTaskType(Task::TaskType type);
public:                                                                 
    TaskFactory();                                                      
    ~TaskFactory();                                                     

    int Initialize(); // Depricated
    int Shutdown(); // Depricated

    // Synchronous versions of methods take care of locking themselves,
    // this method locks and unlocks before completing, making it blocking.
    Task* GetTentTask( Task::TaskType type,                
                       TentApp* pApp,                
                       FileManager* pFm,             
                       CredentialsManager* pCm,      
                       TaskArbiter* pTa,
                       TaskFactory* pTf,
                       const AccessToken& at,        
                       const Entity& entity,    
                       const std::string& filepath,  
                       const std::string& tempdir,   
                       const std::string& workingdir,
                       const std::string& configdir, 
                       TaskDelegate* callbackDelegate,
                       TaskFactoryDelegate* delegate = NULL);

    Task* GetManifestTask( Task::TaskType type,
                           FileManager* pFm,
                           void (*callback)(int, char**, int, int),
                           TaskFactoryDelegate* delegate = NULL);

    int RemoveActiveTask(Task* pTask); // Depricated

    // Information methods
    int GetNumberOfActiveTasks(const Task::TaskType type); // Depricated

    int GetActiveTaskUploadSpeed(); // Depricated
    int GetActiveTaskDownloadSpeed(); // Depricated

private:
    TaskPool    m_ActiveTaskPool;
    TaskPool    m_InactiveTaskPool;
};                                                                      

#endif

