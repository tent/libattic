#include "polling.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "configmanager.h"
#include "censushandler.h"
#include "filesync.h"
#include "foldersync.h"
#include "filesystem.h"
#include "folderhandler.h"
#include "filehandler.h"
#include "filepost.h"
#include "logutils.h"


namespace attic {

static long total_elapsed = 0;
//static boost::timer::nanosecond_type const limit(1 * 1000000000LL); // 1 seconds in nanoseconds
static boost::timer::nanosecond_type const limit(2 * 100000000LL); 

Polling::Polling(FileManager* fm,
                 CredentialsManager* cm,
                 const Entity& entity) {
    file_manager_ = fm;
    credentials_manager_ = cm;
    entity_ = entity;
    running_ = false;


    census_handler_ = NULL;    
    folder_sync_ = NULL;
    file_sync_ = NULL;
    thread_ = NULL;
    state_ = Polling::RUNNING;
}

Polling::~Polling() {}

void Polling::Initialize() {
    if(!thread_) {
        set_running(true);
        //std::cout<<" starting file sync thread ... " << std::endl;
        thread_ = new boost::thread(&Polling::Run, this);
    }
}

void Polling::Shutdown() {
    if(thread_) {
        set_running(false);
        //std::cout<<" exiting file sync thread " << std::endl;
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
}

void Polling::Pause() {
    state_mtx_.Lock();
    state_ = Polling::PAUSED;
    state_mtx_.Unlock();
}

void Polling::Resume() {
    state_mtx_.Lock();
    state_ = Polling::RUNNING;
    state_mtx_.Unlock();
}

void Polling::Run() {
    OnStart();
    while(running_) {
        if(!timer_.is_stopped()) {
            boost::timer::cpu_times time = timer_.elapsed();
            boost::timer::nanosecond_type const elapsed(time.system + time.user);
            total_elapsed += elapsed;
        }
        //std::cout<<" total elapsed : " << total_elapsed << "limit " << limit << std::endl;
        if(total_elapsed > limit) {
            std::cout<<" POLLING - ELAPSED : " << total_elapsed << std::endl;
            total_elapsed = 0;
            timer_.stop();
            state_mtx_.Lock();
            PollState s = state_;
            state_mtx_.Unlock();
            if(s == Polling::RUNNING) {
                // Check deleted folder posts
                PollDeletedFolderPosts();
                // Check folder posts
                PollFolderPosts();
                // Check deleted file posts
                PollDeletedFilePosts();
                // Check all file posts
                PollFilePosts();
                // Check all shared file posts
                PollSharedFilePosts();
            }
            timer_.start();
        }
        sleep::mil(100);
    }
    OnFinished();
}

void Polling::OnStart() {
    //std::cout<<" POLL TASK STARTING " << std::endl;
   
    AccessToken at;
    credentials_manager_->GetAccessTokenCopy(at);

    census_handler_ = new CensusHandler(entity_.GetPreferredServer().posts_feed(), at);
    folder_sync_ = new FolderSync(file_manager_,
                                  at,
                                  entity_.entity(),
                                  entity_.GetPreferredServer().posts_feed(),
                                  entity_.GetPreferredServer().post());
    folder_sync_->Initialize();

    std::string mk;
    GetMasterKey(mk);
    file_sync_ = new FileSync(file_manager_,
                              at,
                              entity_.entity(),
                              entity_.GetPreferredServer().posts_feed(),
                              entity_.GetPreferredServer().post(),
                              mk);
    file_sync_->Initialize();
    timer_.start();
}

void Polling::OnFinished() {
    //std::cout<<" POLL TASK FINISHING " << std::endl;
    timer_.stop();

    if(folder_sync_) {
        folder_sync_->Shutdown();
        delete folder_sync_;
        folder_sync_ = NULL;
    }

    if(file_sync_) { 
        file_sync_->Shutdown();
        delete file_sync_;
        file_sync_ = NULL;
    }

    if(census_handler_) {
        delete census_handler_;
        census_handler_ = NULL;
    }

}

bool Polling::running() {
    bool t;
    r_mtx_.Lock();
    t = running_;
    r_mtx_.Unlock();
    return t;
}

void Polling::set_running(bool r) {
    r_mtx_.Lock();
    running_ = r;
    r_mtx_.Unlock();
}

void Polling::PollFilePosts() {
    //std::cout<<" polling files ... " << std::endl;
    std::deque<FilePost> file_list;
    if(census_handler_->Inquiry("", file_list)) {
        //std::cout<<" Retrieved : " << file_list.size() << " files " << std::endl;
        std::deque<FilePost>::reverse_iterator itr = file_list.rbegin();
        for(;itr != file_list.rend(); itr++) {
            file_sync_->PushBack(*itr);
        }
    }
}

void Polling::PollSharedFilePosts() {
    std::deque<SharedFilePost> shared_list;
    if(census_handler_->Inquiry("", shared_list)) {
        std::deque<SharedFilePost>::reverse_iterator itr = shared_list.rbegin();
        for(;itr != shared_list.rend(); itr++) {

        }
    }
}

void Polling::PollDeletedFilePosts() {
    //std::cout<<" polling deleted files ... " << std::endl;
    std::deque<FilePost> deleted_list;
    if(census_handler_->Inquiry(cnst::g_deleted_fragment, deleted_list)) {
        FileHandler fh(file_manager_);
        std::string master_key;
        if(GetMasterKey(master_key)) {
            std::deque<FileInfo> file_list;

            std::deque<FilePost>::reverse_iterator fp_itr = deleted_list.rbegin();
            for(;fp_itr!=deleted_list.rend(); fp_itr++) {
                FileInfo fi;
                fh.DeserializeIntoFileInfo((*fp_itr), master_key, fi);
                file_list.push_back(fi);
            }
            //std::cout<<" Retreived : " << deleted_list.size() << " deleted files " << std::endl;
            std::deque<FileInfo>::iterator fi_itr = file_list.begin();
            for(;fi_itr!=file_list.end(); fi_itr++) {
                //std::cout<<"deleting ... " << (*fi_itr).filepath() << std::endl;
                DeleteLocalFile(*fi_itr);
            }
        }
    }
}

void Polling::PollDeletedFolderPosts() { 
    //std::cout<<" polling deleted folders ... " << std::endl;
    FolderHandler fh(file_manager_);
    std::deque<FolderPost> folder_list;
    if(census_handler_->Inquiry(cnst::g_deleted_fragment, folder_list)){
        //std::cout<<" Retreived : " << folder_list.size() << " deleted folders " << std::endl;
        std::deque<FolderPost>::reverse_iterator itr = folder_list.rbegin();
        for(;itr != folder_list.rend(); itr++) {
            // Delete local folder
            DeleteLocalFolder(*itr);
        }
    }
}

void Polling::PollFolderPosts() {
    std::ostringstream err;
    err << "************************************************ " << std::endl;
    err <<" polling folder posts ... " << std::endl;
    FolderHandler fh(file_manager_);
    std::deque<FolderPost> folder_list;
    if(census_handler_->Inquiry("", folder_list)){
        err <<" Retreived : " << folder_list.size() << " folders " << std::endl;
        std::deque<FolderPost>::reverse_iterator r_itr = folder_list.rbegin();
        for(;r_itr != folder_list.rend(); r_itr++) {
            err << (*r_itr).folder().foldername() << " id : ";
            err << (*r_itr).id() << " parent_post id : ";
            err << (*r_itr).folder().parent_post_id() << std::endl;
        }
        err << "************************************************ " << std::endl;
        //std::cout << err.str() << std::endl;

        std::deque<FolderPost>::reverse_iterator itr = folder_list.rbegin();
        for(;itr != folder_list.rend(); itr++) {
            // Push back to consumer
            if(folder_sync_) folder_sync_->PushBack(*itr);
        }
    }
}
void Polling::DeleteLocalFile(const FileInfo& fi){ // TODO :: temp method, will move to its own job
    //FolderHandler fh(file_manager_);
    //fh.DeleteFolder(fp.relative_path());
    std::string canonical_path;
    file_manager_->GetCanonicalPath(fi.filepath(), canonical_path);
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

void Polling::DeleteLocalFolder(const FolderPost& fp) {
    // check if folder is in cache
    FolderHandler fh(file_manager_);
    if(!fh.IsFolderInCacheWithId(fp.id())){
        fh.InsertFolder(fp);
    }

    fh.SetFolderDeleted(fp.id(), true);
    // perform local operations
    std::string canonical_path, aliased_path;
    file_manager_->ConstructFolderpath(fp.id(), aliased_path);
    file_manager_->GetCanonicalPath(aliased_path, canonical_path);
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

bool Polling::GetMasterKey(std::string& out) {
        MasterKey mKey;
        credentials_manager_->GetMasterKeyCopy(mKey);
        mKey.GetMasterKey(out);
        if(out.size())
            return true;
        return false;
    }

bool Polling::ValidMasterKey() {
    std::string mk;
    return GetMasterKey(mk);
}

}//namespace

