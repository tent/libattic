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
                         const std::string& configdir) {
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
    if(status == ret::A_OK) {
        status = task_factory_.Initialize();
        if(status == ret::A_OK) {
            event::RegisterForEvent(this, event::Event::REQUEST_PULL);
            event::RegisterForEvent(this, event::Event::REQUEST_PUSH);
            event::RegisterForEvent(this, event::Event::REQUEST_DELETE);
            event::RegisterForEvent(this, event::Event::REQUEST_SYNC_POST);
            //event::RegisterForEvent(this, event::Event::POLL);
        }
    }

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

void TaskManager::UploadFile(const std::string& filepath, TaskDelegate* pDel) {
    TaskContext tc;
    tc.set_value("filepath", filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::PUSH);
    PushContextBack(tc);
}

void TaskManager::DownloadFile(const std::string& filepath, TaskDelegate* pDel) {
    TaskContext tc;
    tc.set_value("filepath", filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::PULL);
    PushContextBack(tc);
}

void TaskManager::SyncFile(const std::string& postid, TaskDelegate* pDel) {
    TaskContext tc;
    tc.set_value("postid", postid);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::SYNC);
    PushContextBack(tc);
}

void TaskManager::DeleteFile(const std::string& filepath, TaskDelegate* pDel) {
    TaskContext tc;
    tc.set_value("filepath", filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::DELETE);
    PushContextBack(tc);
}

void TaskManager::PollFiles(TaskDelegate* pDel) { // This will need to be a direct call
    TaskContext tc;
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::POLL);
    PushContextBack(tc);
}

void TaskManager::RenameFile(const std::string& original_filepath, 
                             const std::string& new_filename) {
    TaskContext tc;
    tc.set_value("file_type", "file");
    tc.set_value("original_filepath", original_filepath);
    tc.set_value("new_filename", new_filename);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::RENAME);
    PushContextBack(tc);
}

void TaskManager::RenameFolder(const std::string& original_folderpath, 
                               const std::string& new_foldername) {
    TaskContext tc;
    tc.set_value("file_type", "folder");
    tc.set_value("original_folderpath", original_folderpath);
    tc.set_value("new_foldername", new_foldername);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::RENAME);
    PushContextBack(tc);
}

void TaskManager::PushContextBack(TaskContext& tc) {
    cxt_mtx.Lock();
    context_queue_.push_back(tc);
    cxt_mtx.Unlock();
}

void TaskManager::RetrieveContextQueue(TaskContext::ContextQueue& out) {
    cxt_mtx.Lock();
    out = context_queue_;
    context_queue_.clear();
    cxt_mtx.Unlock();
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
