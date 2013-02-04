#include "pullalltask.h"

#include <vector>

#include "filemanager.h"

namespace pullall 
{
    static PullAllTask* g_pCurrent = NULL;
}

void PullAllCallBack(int a, void* b)
{
    if(pullall::g_pCurrent)
    {
        pullall::g_pCurrent->PullAllCb(a, b);
    }

}

PullAllTask::PullAllTask( TentApp* pApp, 
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
                                    callback )
{
    m_CallbackCount = 0;
    m_CallbackHit = 0;
}

PullAllTask::~PullAllTask()
{

}

void PullAllTask::OnStart() 
{ 
    std::cout<<" ON START " << std::endl;

} 
void PullAllTask::OnPaused() 
{ 
    //std::cout<<" ON PAUSED " << std::endl;

}

void PullAllTask::OnFinished() 
{ 
    int status = ret::A_OK;
    std::cout<< " ON FINISHED " << std::endl;
    pullall::g_pCurrent = NULL;

    Callback(status, NULL);
    SetFinishedState();
}

void PullAllTask::RunTask()
{
    int status = ret::A_OK;

    if(!pullall::g_pCurrent)
    {
        pullall::g_pCurrent = this;

        // Get Number of Files
        FileManager* pFm = GetFileManager();
        if(pFm)
        {
            std::vector<FileInfo> filelist;          
            pFm->Lock();
            pFm->GetAllFileInfo(filelist);
            pFm->Unlock();

            // Spin off n tasks for each file
            TaskFactory* tf = GetTaskFactory();
            TaskArbiter* ta = GetTaskArbiter();
            std::string entityurl, tempdir, workingdir, configdir;
            GetEntityUrl(entityurl);
            GetTempDirectory(tempdir);
            GetWorkingDirectory(workingdir);
            GetConfigDirectory(configdir);

            Entity entity;
            GetEntity(entity);

            std::string fp;
            std::vector<FileInfo>::iterator itr = filelist.begin();
            for(;itr != filelist.end(); itr++)
            {
                fp.clear();
                (*itr).GetFilepath(fp);

                std::cout<<" PULLING FILE : " << fp << std::endl;
                Task* t = tf->SynchronousGetTentTask( Task::PULL,
                                               GetTentApp(), 
                                               pFm, 
                                               GetCredentialsManager(),
                                               GetTaskArbiter(),
                                               GetTaskFactory(),
                                               GetAccessTokenCopy(),
                                               entity,
                                               fp,               
                                               tempdir,          
                                               workingdir,       
                                               configdir,        
                                               &PullAllCallBack);                

                ta->SpinOffTask(t);
                m_CallbackCount++;
            }

            /*
            //wait
           
            for(;;)
            {
                if(m_CallbackCount <= m_CallbackHit)
                {
                    break;
                }

            }
            */
            SetPausedState();
        }
        else
        {
            status = ret::A_FAIL_INVALID_PTR;
        }

     //   pullall::g_pCurrent = NULL;
    }
    else
    {
        status = ret::A_FAIL_RUNNING_SINGLE_INSTANCE;
    }
                                       
    /*
    Callback(status, NULL);
    SetFinishedState();
    */

    if(status != ret::A_OK)
    {
        Callback(status, NULL);
        SetFinishedState();
    }
}                                      

void PullAllTask::PullAllCb(int a, void* b)
{
    m_CallbackHit++;
}

                                      
                                       
                                       
                                       
                                       
                                       
                                       
                                       
