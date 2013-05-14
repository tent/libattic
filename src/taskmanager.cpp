#include "taskmanager.h"

#include "taskarbiter.h"
#include "tentapp.h"
#include "credentialsmanager.h"
#include "filemanager.h"
#include "taskdelegate.h"

namespace attic {

static unsigned int upload_count = 0;
static unsigned int download_count = 0;
static unsigned int poll_count = 0;
static unsigned int syncfile_count = 0;
static unsigned int delete_count = 0;

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

    task_factory_.Initialize(file_manager_, credentials_manager_, access_token_, entity_);
    
    if(status == ret::A_OK) {
        event::RegisterForEvent(this, event::Event::REQUEST_PULL);
        event::RegisterForEvent(this, event::Event::REQUEST_PUSH);
        event::RegisterForEvent(this, event::Event::REQUEST_DELETE);
        event::RegisterForEvent(this, event::Event::REQUEST_SYNC_POST);
        //event::RegisterForEvent(this, event::Event::POLL);
    }

    return status;
}

int TaskManager::Shutdown() {
    int status = ret::A_OK;

    std::cout<< "TaskManager Stats : " << std::endl;
    std::cout<< "\t upload count : " << upload_count << std::endl;
    std::cout<< "\t download count : " << download_count << std::endl;
    std::cout<< "\t poll count : " << poll_count << std::endl;
    std::cout<< "\t sync file count : " << syncfile_count << std::endl;
    std::cout<< "\t delete count : " << delete_count << std::endl;

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
    std::cout<<" UPLOADING FILE " << std::endl;
    TaskContext tc;
    tc.set_value("filepath", filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::PUSH);
    tc.set_delegate(pDel);
    PushContextBack(tc);
    upload_count++;
}

void TaskManager::DownloadFile(const std::string& filepath, TaskDelegate* pDel) {
    TaskContext tc;
    tc.set_value("filepath", filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::PULL);
    tc.set_delegate(pDel);
    PushContextBack(tc);
    download_count++;
}

void TaskManager::SyncFile(const std::string& postid, TaskDelegate* pDel) {
    TaskContext tc;
    tc.set_value("postid", postid);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::SYNC_FILE_TASK);
    tc.set_delegate(pDel);
    PushContextBack(tc);
    syncfile_count++;
}

void TaskManager::DeleteFile(const std::string& filepath, TaskDelegate* pDel) {
    TaskContext tc;
    tc.set_value("filepath", filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::DELETE);
    tc.set_delegate(pDel);
    PushContextBack(tc);
    delete_count++;
}

void TaskManager::PollFiles(TaskDelegate* pDel) { // This will need to be a direct call
    TaskContext tc;
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::POLL);
    tc.set_delegate(pDel);
    PushContextBack(tc);
    poll_count++;
}

void TaskManager::RenameFile(const std::string& original_filepath, 
                             const std::string& new_filepath) {
    TaskContext tc;
    std::cout<<" RENAME FILE " << std::endl;
    tc.set_value("file_type", "file");
    tc.set_value("original_filepath", original_filepath);
    tc.set_value("new_filepath", new_filepath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::RENAME);
    PushContextBack(tc);
}

void TaskManager::RenameFolder(const std::string& original_folderpath, 
                               const std::string& new_folderpath) {
    TaskContext tc;
    tc.set_value("file_type", "folder");
    tc.set_value("original_folderpath", original_folderpath);
    tc.set_value("new_folderpath", new_folderpath);
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::RENAME);
    PushContextBack(tc);
}

TaskContext TaskManager::CreateServiceContext(void) {
    TaskContext tc;
    tc.set_value("temp_dir", temp_directory_);
    tc.set_value("working_dir", working_directory_);
    tc.set_value("config_dir", config_directory_);
    tc.set_type(Task::SERVICE);
    return tc;
}

void TaskManager::PushContextBack(TaskContext& tc) {
    cxt_mtx.Lock();
    context_queue_.push_back(tc);
    cxt_mtx.Unlock();
}

void TaskManager::RetrieveContextQueue(TaskContext::ContextQueue& out) {
    cxt_mtx.Lock();
//    std::cout<<" context queue size (before) : " << context_queue_.size() << std::endl;
    //std::cout<<" outgoing queue size (before) : " << out.size() << std::endl;
    out = context_queue_;
    context_queue_.clear();
    //std::cout<<" context queue size (after) : " << context_queue_.size() << std::endl;
    //std::cout<<" outgoing queue size (after) : " << out.size() << std::endl;
    cxt_mtx.Unlock();
}

}//namespace
