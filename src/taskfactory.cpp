
#include "taskfactory.h"

#include "tenttask.h"
#include "cryptotask.h"
#include "pulltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "encrypttask.h"

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

    switch(type)
    {
    case PUSH:
        {

        }
    case PULL:
        {

        }
    case DELETE:
        {

        }
    default:
        {

        }
    }

}

Task* TaskFactory::CreateCryptoTask( TaskType type,
                                     const std::string& filepath,
                                     const std::string& outpath,
                                     const Credentials* pCred,
                                     bool generate)
{

    switch(type)
    {

    }

}


