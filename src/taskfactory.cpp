
#include "taskfactory.h"

#include "tenttask.h"
#include "cryptotask.h"
#include "pulltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "encrypttask.h"
#include "syncmanifesttask.h"
#include "syncposttask.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "connectionmanager.h"
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

Task* TaskFactory::CreateTentTask( TaskType type,                                
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
                                   void (*callback)(int, void*))
{

    std::cout<< " Creating tent task ... " << std::endl;

    Task* t = NULL;
    switch(type)
    {
    case PUSH:
        {
            t = new PushTask( pApp,                            
                              pFm,                    
                              pCon,
                              pCm,                    
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
                              pCon,
                              pCm,                    
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
                                pCon,
                                pCm,                    
                                at,
                                entity,                          
                                filepath,                        
                                tempdir,                   
                                workingdir,                
                                configdir,                 
                                callback);       
            break;
        }
    case SYNCMANIFEST:
        {
            t = new SyncManifestTask( pApp,                    
                                      pFm,                     
                                      pCon,                    
                                      pCm,                     
                                      at,                      
                                      entity,                  
                                      filepath,                
                                      tempdir,                 
                                      workingdir,              
                                      configdir,               
                                      callback);               
            break;
        }
    case SYNCPOSTS:
        {
            t = new SyncPostsTask( pApp,                    
                                   pFm,                     
                                   pCon,                    
                                   pCm,                     
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

    if(t)
    {
        m_ActiveTasks.push_back(t);
    }
    
    return t;
}

Task* TaskFactory::CreateCryptoTask( TaskType type,
                                     const std::string& filepath,
                                     const std::string& outpath,
                                     const Credentials* pCred,
                                     bool generate)
{

    switch(type)
    {
    case ENCRYPT:
        {

            break;
        }
    case DECRYPT:
        {

            break;
        }
    default:
        {

        }
    }

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

