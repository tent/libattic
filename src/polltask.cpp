#include "polltask.h"

#include <iostream>

#include "netlib.h"
#include "constants.h"
#include "folderpost.h"
#include "event.h"
#include "taskdelegate.h"
#include "sleep.h"
#include "logutils.h"

#include "filesystem.h"
#include "configmanager.h"

#include "filehandler.h"
#include "folderhandler.h"
#include "renamehandler.h"

namespace attic {

namespace polltask {
    static PollTask* g_pCurrentPollTask = NULL;

    static void PollTaskCB(int a, void* b)
    {
    }
}

static long total_elapsed = 0;
//static boost::timer::nanosecond_type const limit(1 * 1000000000LL); // 1 seconds in nanoseconds
static boost::timer::nanosecond_type const limit(1 * 100000000LL); 

PollTask::PollTask( FileManager* pFm,
                    CredentialsManager* pCm,
                    const AccessToken& at,
                    const Entity& entity,
                    const TaskContext& context)
                    :                                               
                    TentTask(Task::POLL,
                             pFm,
                             pCm,
                             at,
                             entity,                               
                             context) {
    census_handler_ = NULL;    
    delegate_ = new PollDelegate(this);
    running_ = true;
}

PollTask::~PollTask() {
    if(delegate_) {
        delete delegate_;
        delegate_ = NULL;
    }
    if(census_handler_) {
        delete census_handler_;
        census_handler_ = NULL;
    }
}

void PollTask::OnStart(){
    std::cout<<" POLL TASK STARTING " << std::endl;
    if(!polltask::g_pCurrentPollTask) {
        polltask::g_pCurrentPollTask = this;
    }

    event::RegisterForEvent(this, event::Event::PAUSE);
    event::RegisterForEvent(this, event::Event::RESUME);
    Entity entity = TentTask::entity();
    census_handler_ = new CensusHandler(entity.GetPreferredServer().posts_feed(), access_token());
    timer_.start();
}

void PollTask::OnPaused() {}

void PollTask::OnFinished() {
    std::cout<<" POLL TASK FINISHING " << std::endl;
    event::UnregisterFromEvent(this, event::Event::PAUSE);
    event::UnregisterFromEvent(this, event::Event::RESUME);
    timer_.stop();
}

void PollTask::OnEventRaised(const event::Event& event){
    switch(event.type){
        case event::Event::PAUSE:
            running_ = false;
            break;
        case event::Event::RESUME:
            running_ = true;
            break;
        default:
            std::cout<<" Unknown event : " << event.type << std::endl;
    };
}
void PollTask::PollTaskCB(int a, std::string& b) {
    std::cout<<" POLL TASK CALLBACK HIT " << std::endl;
    std::cout<<" CB " << a << " : " << b << std::endl;
}

void PollTask::RunTask() {
    int status = ret::A_OK;
    // Spin off consumer task for checking each file meta post for newer versions
    if(polltask::g_pCurrentPollTask == this) {
        if(!timer_.is_stopped()) {
            boost::timer::cpu_times time = timer_.elapsed();
            boost::timer::nanosecond_type const elapsed(time.system + time.user);
            total_elapsed += elapsed;
        }
        //std::cout<<" total elapsed : " << total_elapsed << "limit " << limit << std::endl;
        if(total_elapsed > limit) {
            std::cout<<" ********************************************************" << std::endl;
            std::cout<<" POLLING - ELAPSED : " << total_elapsed << std::endl;
            std::cout<<" ********************************************************" << std::endl;
            total_elapsed = 0;
            timer_.stop();
            if(running_) {
                // Check deleted folder posts
                PollDeletedFolderPosts();
                // Check folder posts
                PollFolderPosts();
                // Check deleted file posts
                PollDeletedFilePosts();
                // Check all file posts
                PollFilePosts();
            }
            timer_.start();
        }
    }
    else {
        std::cout<<" FAIL MULTI INSTANCE " << std::endl;
        status = ret::A_FAIL_RUNNING_SINGLE_INSTANCE;
        Callback(status, NULL);
        SetFinishedState();
    }

    sleep::sleep_milliseconds(100);
    //sleep::sleep_seconds(2);
}

void PollTask::PollFilePosts() {
    std::cout<<" polling files ... " << std::endl;
    std::deque<FilePost> file_list;
    if(census_handler_->Inquiry("", file_list)) {
        std::cout<<" Retrieved : " << file_list.size() << " files " << std::endl;
        int status = SyncFiles(file_list);
        if(status != ret::A_OK)
            std::cout<<" POLLING ERR : " << status << std::endl;
    }
}

int PollTask::SyncFiles(std::deque<FilePost>& file_list) {
    int status = ret::A_OK;
    // Process Posts
    std::deque<FilePost>::reverse_iterator itr = file_list.rbegin();
    for(;itr != file_list.rend(); itr++) {
        std::cout<<" poll task requesting file sync, id : " << (*itr).id() << std::endl;
        event::RaiseEvent(event::Event::REQUEST_SYNC_POST, 
                          (*itr).id(),
                          delegate_);
    }
    return status;
}
void PollTask::PollDeletedFilePosts() {
    std::cout<<" polling deleted files ... " << std::endl;
    std::deque<FilePost> deleted_list;
    if(census_handler_->Inquiry(cnst::g_deleted_fragment, deleted_list)) {
        FileHandler fh(file_manager());
        std::string master_key;
        if(GetMasterKey(master_key)) {
            std::deque<FileInfo> file_list;

            std::deque<FilePost>::reverse_iterator fp_itr = deleted_list.rbegin();
            for(;fp_itr!=deleted_list.rend(); fp_itr++) {
                FileInfo fi;
                fh.DeserializeIntoFileInfo((*fp_itr), master_key, fi);
                file_list.push_back(fi);
            }
            std::cout<<" Retreived : " << deleted_list.size() << " deleted files " << std::endl;
            std::deque<FileInfo>::iterator fi_itr = file_list.begin();
            for(;fi_itr!=file_list.end(); fi_itr++) {
                std::cout<<"deleting ... " << (*fi_itr).filepath() << std::endl;
                DeleteLocalFile(*fi_itr);
            }
        }
    }
}

void PollTask::PollDeletedFolderPosts() { 
    std::cout<<" polling deleted folders ... " << std::endl;
    FolderHandler fh(file_manager());
    std::deque<FolderPost> folder_list;
    if(census_handler_->Inquiry(cnst::g_deleted_fragment, folder_list)){
        std::cout<<" Retreived : " << folder_list.size() << " deleted folders " << std::endl;
        std::deque<FolderPost>::reverse_iterator itr = folder_list.rbegin();
        for(;itr != folder_list.rend(); itr++) {
            // Delete local folder
            DeleteLocalFolder(*itr);
        }
    }
}

void PollTask::PollFolderPosts() {
    std::cout<<" polling folder posts ... " << std::endl;
    FolderHandler fh(file_manager());
    std::deque<FolderPost> folder_list;
    if(census_handler_->Inquiry("", folder_list)){
        std::cout<<" Retreived : " << folder_list.size() << " folders " << std::endl;
        std::deque<FolderPost>::reverse_iterator itr = folder_list.rbegin();
        for(;itr != folder_list.rend(); itr++) {
            std::cout<<" attempting to validate : " << (*itr).folder().foldername() << std::endl;
            fh.ValidateFolder(*itr);
        }
    }
}



void PollTask::DeleteLocalFile(const FileInfo& fi){ // TODO :: temp method, will move to its own job
    //FolderHandler fh(file_manager());
    //fh.DeleteFolder(fp.relative_path());
    std::string canonical_path;
    file_manager()->GetCanonicalPath(fi.filepath(), canonical_path);
    if(fs::CheckFilepathExists(canonical_path)){
        // Move to trash
        std::string trash_path;
        ConfigManager::GetInstance()->GetValue("trash_path", trash_path);
        if(!trash_path.empty() && fs::CheckFilepathExists(trash_path)) {
            // Move to trash;
            fs::MoveFileToFolder(canonical_path, trash_path);
        }
        else {
            std::string msg = "Invalid trash_path";
            log::LogString("MOA1349", msg);
        }
    }
}

void PollTask::DeleteLocalFolder(const FolderPost& fp) {
    // check if folder is in cache
    FolderHandler fh(file_manager());
    if(!fh.IsFolderInCache(fp.folder().foldername())){
        fh.InsertFolder(fp);
    }

    fh.SetFolderDeleted(fp.folder().foldername(), true);
    // perform local operations
    std::string canonical_path;
    file_manager()->GetCanonicalPath(fp.folder().foldername(), canonical_path);
    if(fs::CheckFilepathExists(canonical_path)){
        // Move to trash
        std::string trash_path;
        ConfigManager::GetInstance()->GetValue("trash_path", trash_path);
        if(!trash_path.empty() && fs::CheckFilepathExists(trash_path)) {
            // Move to trash;
            fs::MoveFileToFolder(canonical_path, trash_path);
        }
        else {
            std::string msg = "Invalid trash_path";
            log::LogString("MMA001001", msg);
        }
    }
}

}//namespace

