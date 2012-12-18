
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
        m_TaskMap.push_back(t);
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


