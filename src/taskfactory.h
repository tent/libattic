#ifndef TASKFACTORY_H_
#define TASKFACTORY_H_
#pragma once

#include <string>
#include <deque>

#include "accesstoken.h"
#include "mutexclass.h"
#include "entity.h"
#include "task.h"

class TentApp;
class FileManager;
class ConnectionManager;
class CredentialsManager;
class Credentials;
class TaskArbiter;


class TaskPool : public MutexClass
{
    typedef std::deque<Task*> TaskQueue;
    typedef std::map<Task::TaskType, TaskQueue> TaskMap;

    TaskQueue::iterator FindTask(Task* pTask, Task::TaskType type)
    {
        TaskQueue::iterator itr = m_TaskMap[type].begin();
        if(pTask)
        {
            for(;itr != m_TaskMap[type].end(); itr++)
            {
                if((*itr) == pTask)
                    break;
            }
        }
        else
            itr = m_TaskMap[type].end();

        return itr;
    }
public:
    TaskPool() {}
    ~TaskPool() {}

    void PushBack(Task* pTask)
    {
        Lock();
        if(pTask)
            m_TaskMap[pTask->GetTaskType()].push_back(pTask);
        Unlock();
    }

    Task* Remove(Task* pTask)
    {
        if(pTask)
        {
            // Find
            TaskQueue::iterator itr = FindTask(pTask, pTask->GetTaskType());
            if(itr != m_TaskMap[pTask->GetTaskType()].end())
            {
                // Remove
                pTask = *itr;                
                *itr = NULL;
            }
        }

        return pTask;
    }
    
private:

    TaskMap m_TaskMap;
};

class TaskFactory : public MutexClass
{                                                                       
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
                             void (*callback)(int, void*));  

public:                                                                 
    TaskFactory();                                                      
    ~TaskFactory();                                                     

    int Initialize();
    int Shutdown();

    // Synchronous versions of methods take care of locking themselves,
    // this method locks and unlocks before completing, making it blocking.
    Task* SynchronousGetTentTask( Task::TaskType type,                
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
                           void (*callback)(int, void*));

    int RemoveActiveTask(Task* pTask);

/*
    Task* GetTentTask( TaskType type,                                
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
                       void (*callback)(int, void*));                
    */

private:
    TaskPool    m_ActiveTaskPool;
    TaskPool    m_InactiveTaskPool;
};                                                                      

#endif

