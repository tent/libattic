#include "taskmanager.h"

#include "taskarbiter.h"
#include "tentapp.h"
#include "credentialsmanager.h"
#include "filemanager.h"
#include "taskdelegate.h"

namespace attic {

TaskManager::TaskManager(FileManager* pFm, 
                         CredentialsManager* pCm,
                         const AccessToken& at,
                         const Entity& entity,
                         const std::string& tempdir, 
                         const std::string& workingdir,
                         const std::string& configdir)
{
    file_manager_ = pFm;
    credentials_manager_ = pCm;

    access_token_ = at;
    entity_ = entity;

    temp_directory_ = tempdir;
    working_directory_ = workingdir;
    config_directory_ = configdir;
}

TaskManager::~TaskManager() {}

int TaskManager::Initialize() {
    int status = ret::A_OK;
    status = task_factory_.Initialize();

    event::RegisterForEvent(this, event::Event::REQUEST_PULL);
    event::RegisterForEvent(this, event::Event::REQUEST_PUSH);
    event::RegisterForEvent(this, event::Event::REQUEST_DELETE);
    event::RegisterForEvent(this, event::Event::REQUEST_SYNC_POST);
    //event::RegisterForEvent(this, event::Event::POLL);

    return status;
}

int TaskManager::Shutdown() {
    int status = ret::A_OK;
    status = task_factory_.Shutdown();

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

int TaskManager::CreateAndSpinOffTask(Task::TaskType tasktype, 
                                      const std::string& filepath, 
                                      TaskDelegate* pDel) {
    int status = ret::A_OK;

    TaskContext tc;
    tc.set_value("filepath", filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);

    Task* t = task_factory_.GetTentTask(tasktype,
                                        file_manager_,
                                        credentials_manager_,
                                        access_token_,
                                        entity_,
                                        tc, 
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

int TaskManager::PollFiles(TaskDelegate* pDel) {
    return CreateAndSpinOffTask( Task::POLL, "", pDel);
}

int TaskManager::QueryManifest(void(*callback)(int, char**, int, int)) {
    int status = ret::A_OK;
    TaskContext tc;
    Task* t = task_factory_.GetManifestTask( Task::QUERYMANIFEST,
                                             file_manager_,
                                             tc,
                                             callback,
                                             this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;
}

int TaskManager::ScanAtticFolder(void(*callback)(int, char**, int, int)) {
    int status = ret::A_OK;
    TaskContext tc;
    Task* t = task_factory_.GetManifestTask(Task::SCANDIRECTORY,
                                            file_manager_,
                                            tc,
                                            callback,
                                            this);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;
}

int TaskManager::TaskCount(const Task::TaskType type) {
    return task_factory_.GetNumberOfActiveTasks(type);
}

}//namespace
