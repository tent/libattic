#include "syncfiletask.h"

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
                            TentTask( Task::SYNC,
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
    // Retrieve metadata
        // compare versions
        // check if file exists
        // if version on the server is newer, pull
        // if file doesn't exist, pull
}


int SyncFileTask::SyncMetaData()
{

}
