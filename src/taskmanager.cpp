#include "taskmanager.h"

#include "taskarbiter.h"
#include "tentapp.h"
#include "credentialsmanager.h"
#include "filemanager.h"
#include "taskdelegate.h"


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

int TaskManager::Initialize() {
    int status = ret::A_OK;
    status = m_TaskFactory.Initialize();

    event::RegisterForEvent(this, event::Event::REQUEST_PULL);
    event::RegisterForEvent(this, event::Event::REQUEST_PUSH);
    event::RegisterForEvent(this, event::Event::REQUEST_DELETE);
    event::RegisterForEvent(this, event::Event::REQUEST_SYNC_POST);
    //event::RegisterForEvent(this, event::Event::POLL);

    return status;
}

int TaskManager::Shutdown() {
    int status = ret::A_OK;
    status = m_TaskFactory.Shutdown();

    return status;
}

void TaskManager::OnEventRaised(const event::Event& event) {
    std::cout<<" TASK MANAGER EVENT RAISED " << std::endl;
    switch(event.type) {
        case event::Event::REQUEST_PULL:
            {
                DownloadFile(event.value, event.delegate);
                break;
            }
        case event::Event::REQUEST_PUSH:
            {
                UploadFile(event.value, event.delegate);
                break;
            }
        case event::Event::REQUEST_DELETE:
            {
                DeleteFile(event.value, event.delegate);
                break;
            }
        case event::Event::REQUEST_SYNC_POST:
            {
                std::cout<<" creating request sync task " << std::endl;
                SyncFile(event.value, event.delegate);
                break;
            }
        default:
            std::cout<<"received unknown event"<<std::endl;
    }

}

void TaskManager::OnTaskCreate(Task* t) {
    std::cout<<" On Task Create " << std::endl;
    // Get Task type
    // if pull or push task
    // extract curl instance ptr
}

void TaskManager::OnTaskInsert(Task* t) {
    std::cout<<" On Task Insert " << std::endl;
    // Maybe move spin off task to spin off?
    //status = TaskArbiter::GetInstance()->SpinOffTask(t);
}

int TaskManager::SyncFile(const std::string& postid, TaskDelegate* pDel) {
    return CreateAndSpinOffTask( Task::SYNC_FILE_TASK,
                                 postid,
                                 pDel);
}

int TaskManager::CreateAndSpinOffTask( Task::TaskType tasktype, 
                                       const std::string& filepath, 
                                       TaskDelegate* pDel)
{
    int status = ret::A_OK;

    Task* t = m_TaskFactory.GetTentTask( tasktype,
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
                                         pDel,
                                         this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;

}

int TaskManager::UploadFile(const std::string& filepath, TaskDelegate* pDel) {
    return CreateAndSpinOffTask( Task::PUSH, filepath, pDel);
}

int TaskManager::DownloadFile(const std::string& filepath, TaskDelegate* pDel) {
    return CreateAndSpinOffTask( Task::PULL, filepath, pDel);
}

int TaskManager::DeleteFile(const std::string& filepath, TaskDelegate* pDel) {
    return CreateAndSpinOffTask( Task::DELETE, filepath, pDel);
}

int TaskManager::SyncFiles(TaskDelegate* pDel) {
    return CreateAndSpinOffTask(Task::SYNC, "", pDel);
}

int TaskManager::PollFiles(TaskDelegate* pDel) {
    return CreateAndSpinOffTask( Task::POLL, "", pDel);
}

int TaskManager::QueryManifest(void(*callback)(int, char**, int, int)) {
    int status = ret::A_OK;

    Task* t = m_TaskFactory.GetManifestTask( Task::QUERYMANIFEST,
                                             m_pFileManager,
                                             callback,
                                             this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;
}

int TaskManager::TaskCount(const Task::TaskType type) {
    return m_TaskFactory.GetNumberOfActiveTasks(type);
}

