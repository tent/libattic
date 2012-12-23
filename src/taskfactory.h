
#ifndef TASKFACTORY_H_
#define TASKFACTORY_H_
#pragma once

#include <string>
#include <deque>

#include "accesstoken.h"

class Task;
class TentApp;
class FileManager;
class ConnectionManager;
class CredentialsManager;
class Credentials;

class TaskFactory
{                                                                       
public:                                                                 
    enum TaskType                                                       
    {                                                                   
        PUSH=0,                                                         
        PULL,                                                           
        DELETE,                                                         
        SYNCMANIFEST,
        SYNCPOSTS,
        ENCRYPT,
        DECRYPT
    };                                                                  

    void TaskFinished(int code, Task* pTask);
public:                                                                 
    TaskFactory();                                                      
    ~TaskFactory();                                                     

    Task* CreateTentTask( TaskType type,                                
                          TentApp* pApp,                                
                          FileManager* pFm,                             
                          ConnectionManager* pCon,                      
                          CredentialsManager* pCm,                      
                          const AccessToken& at,                        
                          const std::string& entity,                    
                          const std::string& filepath,                  
                          const std::string& tempdir,                   
                          const std::string& workingdir,                
                          const std::string& configdir,                 
                          void (*callback)(int, void*));                

    Task* CreateCryptoTask( TaskType type,
                            const std::string& filepath,
                            const std::string& outpath,
                            const Credentials* pCred = NULL,
                            bool generate=true);

private:
    std::map<TaskType, std::deque<Task*> >  m_TaskPool;
    std::deque<Task*>   m_ActiveTasks;

};                                                                      


#endif

