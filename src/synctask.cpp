#include "synctask.h"

#include "atticpost.h"                  
#include "urlparams.h"                  
#include "constants.h"                  
#include "errorcodes.h"                 
#include "utils.h"                      

SyncTask::SyncTask( TentApp* pApp,
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
              void (*callback)(int, void*))
              :                                               
              TentTask( pApp,                                 
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
                        callback)                             
{
}

SyncTask::~SyncTask()
{
}

void SyncTask::RunTask()
{
    int status = ret::A_OK;

    // Sync meta data posts and populate the manifest
    status = SyncMetaData(); 
    
    if(status == ret::A_OK)
    {
        // Run a pull all files afterwards

    }

    Callback(status, NULL);
}

int SyncTask::SyncMetaData()
{
    int status = ret::A_OK;
    
    int postcount = GetAtticPostCount();
                                                                                                  
                                                                                                 
                   
    return status;
}

int SyncTask::GetAtticPostCount()                                                            
{                                                                                                 
    std::string url;
    GetEntity(url);

    // TODO :: make this provider agnostic                                                         
    url += "/tent/posts/count";

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(cnst::g_szAtticPostType));             

    Response response;                                                                            
    AccessToken* at = GetAccessToken();                                                           
    ConnectionManager::GetInstance()->HttpGetWithAuth( url,                                       
                                                       &params,                                   
                                                       response,                                  
                                                       at->GetMacAlgorithm(),                     
                                                       at->GetAccessToken(),                      
                                                       at->GetMacKey(),                           
                                                       true);                                     

    std::cout<< "CODE : " << response.code << std::endl;                                          
    std::cout<< "RESPONSE : " << response.body << std::endl;                                      

    int count = -1;                                                                               
    if(response.code == 200)
    {
        count = atoi(response.body.c_str());                                                          
    }

    return count;
}    
