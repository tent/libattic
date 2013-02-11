#include "taskmanager.h"

#include "taskarbiter.h"
#include "tentapp.h"
#include "credentialsmanager.h"
#include "filemanager.h"

TaskManager::TaskManager( TentApp* pApp, 
                          FileManager* pFm, 
                          CredentialsManager* pCm,
                          const AccessToken& at,
                          const Entity& entity,
                          const std::string& tempdir, 
                          const std::string& workingdir,
                          const std::string& configdir
                        )
{
    m_pApp = pApp;
    m_pFileManager = pFm;
    m_pCredentialsManager = pCm;

    m_AccessToken = at;
    m_Entity = entity;

    m_TempDir = tempdir;
    m_WorkingDir = workingdir;
    m_ConfigDir = configdir;
}

TaskManager::~TaskManager()
{

}

int TaskManager::Initialize()
{
    int status = ret::A_OK;
    status = m_TaskFactory.Initialize();

    return status;
}

int TaskManager::Shutdown()
{
    int status = ret::A_OK;
    status = m_TaskFactory.Shutdown();

    return status;
}

void TaskManager::OnTaskCreate(Task* t)
{
    std::cout<<" On Task Create " << std::endl;
    // Get Task type
    // if pull or push task
    // extract curl instance ptr
}

void TaskManager::OnTaskInsert(Task* t)
{
    std::cout<<" On Task Insert " << std::endl;

    // Maybe move spin off task to spin off?
    //status = TaskArbiter::GetInstance()->SpinOffTask(t);
}

int TaskManager::UploadFile(const std::string& filepath, void (*callback)(int, void*))
{
    int status = ret::A_OK;

    Task* t = m_TaskFactory.GetTentTask( Task::PUSH,
                                                    m_pApp,
                                                    m_pFileManager,
                                                    m_pCredentialsManager,
                                                    TaskArbiter::GetInstance(),
                                                    &m_TaskFactory,
                                                    m_AccessToken,
                                                    m_Entity,
                                                    filepath,
                                                    m_TempDir,
                                                    m_WorkingDir,
                                                    m_ConfigDir,
                                                    callback,
                                                    this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;
}

int TaskManager::DownloadFile(const std::string& filepath, void (*callback)(int, void*))
{
    int status = ret::A_OK;
 
    Task* t = m_TaskFactory.GetTentTask( Task::PULL,
                                                    m_pApp,
                                                    m_pFileManager,
                                                    m_pCredentialsManager,
                                                    TaskArbiter::GetInstance(),
                                                    &m_TaskFactory,
                                                    m_AccessToken,
                                                    m_Entity,
                                                    filepath,
                                                    m_TempDir,
                                                    m_WorkingDir,
                                                    m_ConfigDir,
                                                    callback,
                                                    this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;
}

int TaskManager::DeleteFile(const std::string& filepath, void (*callback)(int, void*))
{
    int status = ret::A_OK;

    Task* t = m_TaskFactory.GetTentTask( Task::DELETE,
                                                    m_pApp,
                                                    m_pFileManager,
                                                    m_pCredentialsManager,
                                                    TaskArbiter::GetInstance(),
                                                    &m_TaskFactory,
                                                    m_AccessToken,
                                                    m_Entity,
                                                    filepath,
                                                    m_TempDir,
                                                    m_WorkingDir,
                                                    m_ConfigDir,
                                                    callback,
                                                    this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);

    return status;
}

int TaskManager::DownloadAllFiles(void (*callback)(int, void*))
{
    int status = ret::A_OK;

    Task* t = m_TaskFactory.GetTentTask( Task::PULLALL,
                                                    m_pApp,
                                                    m_pFileManager,
                                                    m_pCredentialsManager,
                                                    TaskArbiter::GetInstance(),
                                                    &m_TaskFactory,
                                                    m_AccessToken,
                                                    m_Entity,
                                                    "",
                                                    m_TempDir,
                                                    m_WorkingDir,
                                                    m_ConfigDir,
                                                    callback,
                                                    this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;
}

int TaskManager::SyncFiles(void (*callback)(int, void*))
{
    int status = ret::A_OK;

    Task* t = m_TaskFactory.GetTentTask( Task::SYNC,
                                                    m_pApp,
                                                    m_pFileManager,
                                                    m_pCredentialsManager,
                                                    TaskArbiter::GetInstance(),
                                                    &m_TaskFactory,
                                                    m_AccessToken,
                                                    m_Entity,
                                                    "",
                                                    m_TempDir,
                                                    m_WorkingDir,
                                                    m_ConfigDir,
                                                    callback,
                                                    this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);

    return status;
}

int TaskManager::DeleteAllPosts(void (*callback)(int, void*))
{
    int status = ret::A_OK;

    Task* t = m_TaskFactory.GetTentTask( Task::DELETEALLPOSTS,
                                                    m_pApp,
                                                    m_pFileManager,
                                                    m_pCredentialsManager,
                                                    TaskArbiter::GetInstance(),
                                                    &m_TaskFactory,
                                                    m_AccessToken,
                                                    m_Entity,
                                                    "",
                                                    m_TempDir,
                                                    m_WorkingDir,
                                                    m_ConfigDir,
                                                    callback,
                                                    this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);

    return status;
}

int TaskManager::QueryManifest(const int stride, void(*callback)(char**, int, int))
{
    int status = ret::A_OK;

    return status;
}

