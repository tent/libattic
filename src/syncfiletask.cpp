#include "syncfiletask.h"

#include "response.h"
#include "urlparams.h"
#include "netlib.h"
#include "jsonserializable.h"


SyncFileTask::SyncFileTask( TentApp* pApp,
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
                            :                                               
                            TentTask( Task::SYNC_FILE_TASK,
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
                                      callback)                             
{
    // TODO :: refine constructor params, FOR NOW filepath will need to have the postid
    m_PostID = filepath;
}

SyncFileTask::~SyncFileTask()
{
}

void SyncFileTask::OnStart()
{
}

void SyncFileTask::OnPaused()
{
}

void SyncFileTask::OnFinished()
{
}

void SyncFileTask::RunTask()
{
    int status = ret::A_OK;

    // Retrieve metadata
    AtticPost p;
    status = SyncMetaData(p);
    if(status == ret::A_OK) {
        std::string filepath;
        p.GetAtticPostFilepath(filepath);
        int version = p.GetVersion();

        std::cout<<" VERSION : " << version << std::endl;
        // compare versions
        // check if file exists
        // if version on the server is newer, pull
        // if file doesn't exist, pull
    }

    Callback(status, NULL);
    SetFinishedState();
}

int SyncFileTask::SyncMetaData(AtticPost& out)
{
    int status = ret::A_OK;

    std::string url;
    GetEntityUrl(url);
    utils::CheckUrlAndAppendTrailingSlash(url);
    url += "posts/" + m_PostID;

    Response response;
    AccessToken* at = GetAccessToken();                                                
    netlib::HttpGet( url,
                     NULL,
                     at,
                     response); 

    std::cout<< " CODE : " << response.code << std::endl;
    std::cout<< " RESPONSE : " << response.body << std::endl;                          

    if(response.code == 200) {
        jsn::DeserializeObject(&out, response.body);
    }
    else
        status = ret::A_FAIL_NON_200;


    return status;
}
