#include "pullalltask.h"

#include <vector>

#include "filemanager.h"

static PullAllTask* g_pCurrent = NULL;

void PullAllCallBack(int a, void* b)
{
    if(g_pCurrent)
    {
        g_pCurrent->PullAllCb(a, b);
    }

}

PullAllTask::PullAllTask( TentApp* pApp, 
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
                                    callback )
{
    m_CallbackCount = 0;
    m_CallbackHit = 0;
}

PullAllTask::~PullAllTask()
{

}

void PullAllTask::RunTask()
{
    int status = ret::A_OK;
    std::cout<<"HAAAAAAAAAAAAAAAARE"<<std::endl;

    if(!g_pCurrent)
    {
        g_pCurrent = this;

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
            GetEntity(entityurl);
            GetTempDirectory(tempdir);
            GetWorkingDirectory(workingdir);
            GetConfigDirectory(configdir);

            std::string fp;
            std::vector<FileInfo>::iterator itr = filelist.begin();
            for(;itr != filelist.end(); itr++)
            {

                std::cout<<" PULLING FILE : " << fp << std::endl;
                fp.clear();
                (*itr).GetFilepath(fp);
                Task* t = tf->SyncGetTentTask( TaskFactory::PULL,
                                               GetTentApp(), 
                                               pFm, 
                                               GetCredentialsManager(),
                                               GetTaskArbiter(),
                                               GetTaskFactory(),
                                               GetAccessTokenCopy(),
                                               entityurl,
                                               fp,               
                                               tempdir,          
                                               workingdir,       
                                               configdir,        
                                               &PullAllCallBack);                

                ta->SpinOffTask(t);
                m_CallbackCount++;
            }

            //wait
            for(;;)
            {
                if(m_CallbackCount <= m_CallbackHit)
                {
                    break;
                }

            }
        }
        else
        {
            status = ret::A_FAIL_INVALID_PTR;
        }

        g_pCurrent = NULL;
    }
    else
    {
        status = ret::A_FAIL_RUNNING_SINGLE_INSTANCE;
    }
                                       
    Callback(status, NULL);
}                                      

void PullAllTask::PullAllCb(int a, void* b)
{
    m_CallbackHit++;
}

                                      
                                       
                                       
                                       
                                       
                                       
                                       
                                       
