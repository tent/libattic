
#include "taskfactory.h"

#include "tenttask.h"
#include "pulltask.h"
#include "pullalltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "synctask.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "connectionmanager.h"
#include "taskarbiter.h"
#include "tentapp.h"

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


    return ret::A_OK;
}

Task* TaskFactory::GetTentTask( TaskType type,                                
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

    std::cout<< " Creating tent task ... " << std::endl;
    std::cout<< " TASK TYPE : " << type << std::endl;

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

    if(t)
        m_ActiveTasks.push_back(t);
    
    return t;
}

Task* TaskFactory::SynchronousGetTentTask( TaskType type,                
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

    if(t)
    {
        Lock();
        m_ActiveTasks.push_back(t);
        Unlock();
    }
    
    return t;
}


Task* TaskFactory::CreateNewTentTask( TaskType type,                  
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
    case PUSH:
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
    case PULL:
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
    case PULLALL:
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
    case DELETE:
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
    case SYNC:
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

        }
    }


    return t;
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

