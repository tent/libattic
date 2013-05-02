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

    Task* CreateNewTentTask(int type,
                            FileManager* pFm,               
                            CredentialsManager* pCm,        
                            const AccessToken& at,          
                            const Entity& entity,      
                            const TaskContext& context);

    Task* CreateNewManifestTask(Task::TaskType type,
                                FileManager* pFm,
                                const TaskContext& context,
                                void (*callback)(int, char**, int, int));

    void LogUnknownTaskType(Task::TaskType type);
public:                                                                 
    TaskFactory();                                                      
    ~TaskFactory();                                                     

    // Synchronous versions of methods take care of locking themselves,
    // this method locks and unlocks before completing, making it blocking.
    Task* GetTentTask(int type,
                      FileManager* pFm,             
                      CredentialsManager* pCm,      
                      const AccessToken& at,        
                      const Entity& entity,    
                      const TaskContext& context);

    Task* GetManifestTask(Task::TaskType type,
                          FileManager* pFm,
                          const TaskContext& context,
                          void (*callback)(int, char**, int, int));
private:
};                                                                      

}//namespace
#endif

