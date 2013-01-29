
#ifndef TASKFACTORY_H_
#define TASKFACTORY_H_
#pragma once

#include <string>
#include <deque>

#include "accesstoken.h"
#include "mutexclass.h"

class Task;
class TentApp;
class FileManager;
class ConnectionManager;
class CredentialsManager;
class Credentials;
class TaskArbiter;

class TaskFactory : public MutexClass
{                                                                       
public:                                                                 
    enum TaskType                                                       
    {                                                                   
        PUSH=0,                                                         
        PULL,
        PULLALL,
        DELETE,                                                         
        SYNCPOSTS,
        ENCRYPT,
        DECRYPT
    };                                                                  

    void TaskFinished(int code, Task* pTask);
    Task* CreateNewTentTask( TaskType type,                  
                             TentApp* pApp,                  
                             FileManager* pFm,               
                             CredentialsManager* pCm,        
                             TaskArbiter* pTa,
                             TaskFactory* pTf,
                             const AccessToken& at,          
                             const std::string& entity,      
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
    
    Task* SynchronousGetTentTask( TaskType type,                
                           TentApp* pApp,                
                           FileManager* pFm,             
                           CredentialsManager* pCm,      
                           TaskArbiter* pTa,
                           TaskFactory* pTf,
                           const AccessToken& at,        
                           const std::string& entity,    
                           const std::string& filepath,  
                           const std::string& tempdir,   
                           const std::string& workingdir,
                           const std::string& configdir, 
                           void (*callback)(int, void*));

    Task* GetTentTask( TaskType type,                                
                       TentApp* pApp,                                
                       FileManager* pFm,                             
                       CredentialsManager* pCm,                      
                       TaskArbiter* pTa,
                       TaskFactory* pTf,
                       const AccessToken& at,                        
                       const std::string& entity,                    
                       const std::string& filepath,                  
                       const std::string& tempdir,                   
                       const std::string& workingdir,                
                       const std::string& configdir,                 
                       void (*callback)(int, void*));                

private:
    typedef std::deque<Task*> TaskPool;
    typedef std::map<TaskType, TaskPool> TaskMap;

    TaskMap     m_TaskPool;
    TaskPool    m_ActiveTasks;

};                                                                      


#endif

