
#include "taskfactory.h"

#include "tenttask.h"
#include "pulltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "queryfilestask.h"
#include "polltask.h"
#include "syncfiletask.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "taskarbiter.h"
#include "tentapp.h"
#include "scandirectorytask.h"

namespace attic { 

TaskFactory::TaskFactory() {}
TaskFactory::~TaskFactory() {}

int TaskFactory::Initialize() // Depricated
{
    std::cout<<" Initializing task factory " << std::endl;


    return ret::A_OK;
}

int TaskFactory::Shutdown() // Depricated
{
    std::cout<<" Shutting down task factory " << std::endl;

    return ret::A_OK;
}

void TaskFactory::PushBackTask(Task* t, TaskFactoryDelegate* delegate)
{
    if(t) {
        if(delegate)
            delegate->OnTaskCreate(t);

        // TODO :: this is probably unecessary, let the owner delete task
        /*
        Lock();
        m_ActiveTaskPool.PushBack(t);
        Unlock();
        */

        if(delegate)
            delegate->OnTaskInsert(t);
    }
}

Task* TaskFactory::GetTentTask( Task::TaskType type,                
                                FileManager* pFm,             
                                CredentialsManager* pCm,      
                                const AccessToken& at,        
                                const Entity& entity,    
                                const std::string& filepath,  
                                const std::string& tempdir,   
                                const std::string& workingdir,
                                const std::string& configdir, 
                                TaskDelegate* callbackDelegate,
                                TaskFactoryDelegate* delegate)

{
    // Check Inactive Task Pool
    // 
    // Otherwise create a new task
    Task* t = NULL;
    t = CreateNewTentTask( type,
                           pFm,       
                           pCm,
                           at,
                           entity,    
                           filepath,  
                           tempdir,   
                           workingdir,
                           configdir, 
                           callbackDelegate); 

    PushBackTask(t, delegate);
    return t;
}


Task* TaskFactory::GetManifestTask( Task::TaskType type,
                                    FileManager* pFm,
                                    void (*callback)(int, char**, int, int),
                                    TaskFactoryDelegate* delegate)
{
    Task* t = NULL;

    t = CreateNewManifestTask( type,
                               pFm,
                               callback);

    PushBackTask(t, delegate);

    return t;
}

Task* TaskFactory::CreateNewManifestTask( Task::TaskType type,
                                          FileManager* pFm,
                                          void (*callback)(int, char**, int, int))
{
    Task* t = NULL;
    switch(type) {
        case Task::QUERYMANIFEST:
        {
            t = new QueryFilesTask(pFm,
                                   callback);
            break;
        }
        case Task::SCANDIRECTORY:
        {
            t = new ScanDirectoryTask(pFm, callback);

            break;
        }
        default:
        {
            LogUnknownTaskType(type);
            break;
        }
    }

    return t;
}

Task* TaskFactory::CreateNewTentTask( Task::TaskType type,                  
                                      FileManager* pFm,               
                                      CredentialsManager* pCm,        
                                      const AccessToken& at,          
                                      const Entity& entity,      
                                      const std::string& filepath,    
                                      const std::string& tempdir,     
                                      const std::string& workingdir,  
                                      const std::string& configdir,   
                                      TaskDelegate* callbackDelegate)
{

    Task* t = NULL;
    switch(type)
    {
        case Task::PUSH:
        {
            t = new PushTask( pFm,
                              pCm,                    
                              at,
                              entity,                          
                              filepath,                        
                              tempdir,                   
                              workingdir,                
                              configdir,                 
                              callbackDelegate);                         
            break;
        }
        case Task::PULL:
        {
            t = new PullTask( pFm,                     
                              pCm,          
                              at,
                              entity,                           
                              filepath,                         
                              tempdir,                    
                              workingdir,                 
                              configdir,                  
                              callbackDelegate);         
            break;
        }
        case Task::DELETE:
        {
            t = new DeleteTask( pFm,                          
                                pCm,          
                                at,
                                entity,                                
                                filepath,                              
                                tempdir,                         
                                workingdir,                      
                                configdir,                       
                                callbackDelegate);             
            break;
        }
        case Task::SYNC:
        {
            std::cout<<" called to create sync task ...why?" << std::endl;
            break;
        }
        case Task::SYNC_FILE_TASK:
        {
            t = new SyncFileTask( pFm,                   
                                  pCm,                   
                                  at,                    
                                  entity,                
                                  filepath,              
                                  tempdir,               
                                  workingdir,            
                                  configdir,             
                                  callbackDelegate);

            break;
        }
        case Task::POLL:
        {
            t = new PollTask( pFm,                   
                              pCm,                   
                              at,                    
                              entity,                
                              filepath,              
                              tempdir,               
                              workingdir,            
                              configdir,             
                              callbackDelegate);             
            break;
        }
        default:
        {
            std::cout<<" CREATING UNKNOWN TASK " << std::endl;
            LogUnknownTaskType(type);
        }
    }


    return t;
}

void TaskFactory::LogUnknownTaskType(Task::TaskType type)
{
    char buf[256] = {'\0'};
    sprintf(buf, "%d", type);
    std::string a = "Unknown task type : ";
    a.append(buf);
}

int TaskFactory::RemoveActiveTask(Task* pTask) // Depricated
{
    int status = ret::A_OK;
    // Remove from active list


    return status;
}

void TaskFactory::TaskFinished(int code, Task* pTask)
{
    if(code == ret::A_OK && pTask) {
        // Reset task and return it into the active pool
    }
    else {
        // Log error
        if(pTask) {
            delete pTask;
            pTask = NULL;
        }
    }
}

int TaskFactory::GetNumberOfActiveTasks(const Task::TaskType type) // Depricated
{
    int taskcount = -1;

    /*
    m_ActiveTaskPool.Lock();
    taskcount = m_ActiveTaskPool[type]->size();
    m_ActiveTaskPool.Unlock();
    */

    return taskcount;
}

}//namespace

