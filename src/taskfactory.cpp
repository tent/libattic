
#include "taskfactory.h"

#include "tenttask.h"
#include "pulltask.h"
#include "pullalltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "deleteallpoststask.h"
#include "synctask.h"
#include "queryfilestask.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "connectionmanager.h"
#include "taskarbiter.h"
#include "tentapp.h"

#include "log.h"

TaskFactory::TaskFactory()
{

}

TaskFactory::~TaskFactory()
{

}

int TaskFactory::Initialize()
{
    std::cout<<" Initializing task factory " << std::endl;


    return ret::A_OK;
}

int TaskFactory::Shutdown()
{
    std::cout<<" Shutting down task factory " << std::endl;

    /*
    TaskMap::iterator itr = m_TaskPool.begin();

    for(;itr != m_TaskPool.end(); itr++)
    {
        TaskPool* pPool = &(itr->second);
        TaskPool::iterator ii = pPool->begin();

        for(;pPool->size() > 0;)
        {
            std::cout<<" deleting task ... " << std::endl;
            Task* pTask = pPool->front();
            delete pTask;
            pTask = NULL;
            pPool->pop_front();
        }
    }

    // TODO :: when pools come into use remove the following ...
    TaskPool::iterator ii = m_ActiveTasks.begin();

    while(m_ActiveTasks.size() > 0)
    {
        Task* pTask = m_ActiveTasks.front();
        std::cout<<" deleting task ... " << std::endl;

        delete pTask;
        pTask = NULL;

        m_ActiveTasks.pop_front();
    }
    */


    return ret::A_OK;
}

void TaskFactory::PushBackTask(Task* t, TaskFactoryDelegate* delegate)
{
    if(t)
    {
        if(delegate)
            delegate->OnTaskCreate(t);

        Lock();
        m_ActiveTaskPool.PushBack(t);
        Unlock();

        if(delegate)
            delegate->OnTaskInsert(t);
    }
}

Task* TaskFactory::GetTentTask( Task::TaskType type,                
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
                                void (*callback)(int, void*),
                                TaskFactoryDelegate* delegate)

{
    // Check Inactive Task Pool
    // 
    // Otherwise create a new task
    Task* t = NULL;
    t = CreateNewTentTask( type,
                           pApp,      
                           pFm,       
                           pCm,
                           pTa,
                           pTf,
                           at,
                           entity,    
                           filepath,  
                           tempdir,   
                           workingdir,
                           configdir, 
                           callback); 

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
    switch(type)
    {
        case Task::QUERYMANIFEST:
        {
            t = new QueryFilesTask( type,
                                    pFm,
                                    callback);
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
                                      void (*callback)(int, void*))
{

    Task* t = NULL;
    switch(type)
    {
        case Task::PUSH:
        {
            t = new PushTask( pApp,                            
                              pFm,                    
                              pCm,                    
                              pTa,
                              pTf,
                              at,
                              entity,                          
                              filepath,                        
                              tempdir,                   
                              workingdir,                
                              configdir,                 
                              callback);                         
            break;
        }
        case Task::PULL:
        {
            t = new PullTask( pApp,                             
                              pFm,                     
                              pCm,          
                              pTa,                     
                              pTf,
                              at,
                              entity,                           
                              filepath,                         
                              tempdir,                    
                              workingdir,                 
                              configdir,                  
                              callback);         
            break;
        }
        case Task::PULLALL:
        {
            t = new PullAllTask( pApp,                             
                                 pFm,                     
                                 pCm,          
                                 pTa,                     
                                 pTf,
                                 at,
                                 entity,                           
                                 filepath,                         
                                 tempdir,                    
                                 workingdir,                 
                                 configdir,                  
                                 callback);   
            break;
        }
        case Task::DELETE:
        {
            t = new DeleteTask( pApp,                                  
                                pFm,                          
                                pCm,          
                                pTa,                          
                                pTf,
                                at,
                                entity,                                
                                filepath,                              
                                tempdir,                         
                                workingdir,                      
                                configdir,                       
                                callback);             
            break;
        }
        case Task::DELETEALLPOSTS:
        {
            t = new DeleteAllPostsTask( pApp,                                  
                                pFm,                          
                                pCm,          
                                pTa,                          
                                pTf,
                                at,
                                entity,                                
                                filepath,                              
                                tempdir,                         
                                workingdir,                      
                                configdir,                       
                                callback);     

            break;
        }
        case Task::SYNC:
        {
            t = new SyncTask( pApp,                  
                              pFm,                   
                              pCm,                   
                              pTa,                   
                              pTf,
                              at,                    
                              entity,                
                              filepath,              
                              tempdir,               
                              workingdir,            
                              configdir,             
                              callback);             
            break;
        }
        default:
        {
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

    alog::Log(Logger::ERROR, a);
}

int TaskFactory::RemoveActiveTask(Task* pTask)
{
    int status = ret::A_OK;
    // Remove from active list


    return status;
}

void TaskFactory::TaskFinished(int code, Task* pTask)
{
    if(code == ret::A_OK && pTask)
    {
        // Reset task and return it into the active pool
    }
    else
    {
        // Log error
        if(pTask)
        {
            delete pTask;
            pTask = NULL;
        }
    }
}

