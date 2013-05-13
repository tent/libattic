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

namespace attic { 

class FileManager;
class CredentialsManager;
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

    Task* CreateNewTentTask(const TaskContext& context);
    Task* CreateNewManifestTask(const TaskContext& context,
                                void (*callback)(int, char**, int, int));

    void LogUnknownTaskType(int type);
public:                                                                 
    TaskFactory();                                                      
    ~TaskFactory();                                                     

    void Initialize(FileManager* fm,
                    CredentialsManager* cm,
                    const AccessToken& at,        
                    const Entity& ent);
    void Shutdown();

    // Synchronous versions of methods take care of locking themselves,
    // this method locks and unlocks before completing, making it blocking.
    Task* GetTentTask(const TaskContext& context);

    Task* GetManifestTask(Task::TaskType type,
                          FileManager* pFm,
                          const TaskContext& context,
                          void (*callback)(int, char**, int, int));

    void ReclaimTask(Task* task);
private:
    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;
    AccessToken             access_token_;
    Entity                  entity_;
};                                                                      

}//namespace
#endif

