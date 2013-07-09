#include "foldersync.h"
#include "sleep.h"

#include "folderhandler.h"
#include "logutils.h"
#include "filesystem.h"
#include "renamehandler.h"

namespace attic {

FolderSync::FolderSync(FileManager* fm, 
                       const AccessToken at,
                       const std::string& entity_url,
                       const std::string& posts_feed,
                       const std::string& post_path) {
    running_ = false;
    at_ = at;
    file_manager_ = fm;
    // avoid cow
    posts_feed_.append(posts_feed.c_str(), posts_feed.size());
    post_path_.append(post_path.c_str(), post_path.size());
    entity_url_.append(entity_url.c_str(), entity_url.size());
    utils::FindAndReplace(post_path, "{entity}", entity_url_, post_path_);
    thread_ = NULL;
}

FolderSync::~FolderSync() {
    file_manager_ = NULL;
}

void FolderSync::Initialize() {
    if(!thread_) {
        running_ = true;
        std::cout<<" STarting FolderSync Thread ... " << std::endl;
        thread_ = new boost::thread(&FolderSync::Run, this);
    }
}

void FolderSync::Shutdown() {
    if(thread_) {
        set_running(false);
        std::cout<<" exiting worker thread .. " << std::endl;
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
}

void FolderSync::Run() {
    std::cout<<" folder sync running .. " << std::endl;
    while(running()) {
        FolderPost fp;
        if(PopFront(fp)) {
            std::cout<<" validating folder tree for : " << fp.folder().foldername() << std::endl;
            if(ValidateFolderTree(fp)) {
                std::cout<<" validating folder : " << fp.folder().foldername() << std::endl;
                ValidateFolder(fp);
            }
        }
        else {
            sleep::mil(100);
        }
    }
    std::cout<<" Folder Sync finishing..." << std::endl;
}

void FolderSync::ValidateFolder(FolderPost& fp) {
    FolderHandler fh(file_manager_);
    if(file_manager_->DoesFolderExistById(fp.folder().parent_post_id())) {
        if(!file_manager_->DoesFolderExistById(fp.id())) {
            if(fh.InsertFolder(fp)) CreateDirectoryTree(fp);
        }
        else {
            RenameHandler rh(file_manager_);
            if(!rh.CheckForRename(fp)) {
                // Check if folder was previously deleted
                std::cout << " no folder rename ... " << std::endl;
                if(fh.IsFolderDeleted(fp.id())) {
                    std::cout<<" is folder deleted yes " << std::endl;
                    if(fp.type().find(cnst::g_deleted_fragment) == std::string::npos) {
                        // un-delete
                        std::cout<<" undeleted " << std::endl;
                        fh.SetFolderDeleted(fp.id(), false);
                    }
                }
                // Make sure path exists anyway
                CreateDirectoryTree(fp);
            }
        }
    }
}

bool FolderSync::ValidateFolderTree(const FolderPost& fp) {
    bool ret = true;
    std::cout<<" Validing folder tree for post : " << fp.id() << std::endl;
    std::cout<<" foldername : " << fp.folder().foldername() << std::endl;
    FolderHandler fh(file_manager_);
    std::string post_id = fp.id();
    while(!file_manager_->IsRootDirectory(post_id)) {
        if(file_manager_->DoesFolderExistById(post_id)) {
            ret = file_manager_->GetFolderParentId(post_id, post_id);
        }
        else {
            // check map for post
            FolderPost folder_post;
            if(RetrievePostFromMap(post_id, folder_post)) {
                ret = fh.InsertFolder(folder_post);
            }
            else {
                
                if(fh.RetrieveFolderPost(post_id, 
                                         entity_url_,
                                         post_path_,
                                         at_,
                                         folder_post) == ret::A_OK) {
                    ret = fh.InsertFolder(folder_post);
                }
                else {
                    ret = false;
                    break;
                }
            }
        }
    }
    return ret;
}

bool FolderSync::RetrievePostFromMap(const std::string& post_id, FolderPost& out) {
    bool ret = false;
    pm_mtx_.Lock();
    PostMap::iterator itr = post_map_.find(post_id);
    if(itr != post_map_.end()) {
        out = itr->second;
        ret = true;
    }
    pm_mtx_.Unlock();
    return ret;
}

void FolderSync::PushBack(const FolderPost& p) {
    pm_mtx_.Lock();
    post_map_[p.id()] = p;
    pm_mtx_.Unlock();
}

bool FolderSync::PopFront(FolderPost& out) {
    bool ret = false;
    pm_mtx_.Lock();
    if(post_map_.size()) {
        out = post_map_.begin()->second;
        // remove
        post_map_.erase(post_map_.begin());
        ret = true;
    }
    pm_mtx_.Unlock();
    return ret;
}

bool FolderSync::running() {
    bool t;
    r_mtx_.Lock();
    t = running_;
    r_mtx_.Unlock();
    return t;
}

void FolderSync::set_running(bool r) {
    r_mtx_.Lock();
    running_ = r;
    r_mtx_.Unlock();
}

bool FolderSync::CreateDirectoryTree(const FolderPost& fp) {
    bool ret = false;
    if(!file_manager_->IsFolderDeleted(fp.id())){
        std::string folderpath, full_folderpath;
        if(ConstructFolderpath(fp, folderpath)) {
            std::cout <<" folder path : " << folderpath << std::endl;
            if(file_manager_->GetCanonicalPath(folderpath, full_folderpath)) { 
                std::cout <<" creating directory tree for " << full_folderpath << std::endl;
                try {
                    //create folder
                    fs::CreateDirectoryTreeForFolder(full_folderpath);
                    ret = true;
                }
                catch(std::exception& e) {
                    log::LogException("fh_1281jn1", e);
                }
            }
        }
        else {
            std::cout << " failed to create directory tree for id : " << fp.id() << std::endl;
        }

        if(full_folderpath.empty()) {
            std::ostringstream error;
            error << "Validate Folder, full folderpath empty ";
            error << " post id : " << fp.id() << std::endl;
            error << " foldername : " << fp.folder().foldername() << std::endl;
            log::LogString("folder_handler_12904", error.str());
        }
    }
    else {
        std::cout<<" CreateDirectoryTree folder is deleted "<< fp.folder().foldername() << std::endl;
        std::cout<<" ret : " << ret << std::endl;
    }
    return ret;
}

bool FolderSync::ConstructFolderpath(const FolderPost& fp, std::string& path_out) {
    bool ret = false;
    std::string post_id = fp.id();
    for(;;) {
        Folder folder;
        if(RetrieveFolder(post_id, folder)) {
            if(!file_manager_->IsRootDirectory(post_id)) {
                if(folder.has_alias())
                    path_out = "/" + folder.local_alias() + path_out;
                else
                    path_out = "/" + folder.foldername() + path_out;
            }
            else {
                if(folder.has_alias())
                    path_out = folder.local_alias() + path_out;
                else
                    path_out = folder.foldername() + path_out;
                ret = true;
                break;
            }
            post_id = folder.parent_post_id();
        }
        else
            break;
    }
    return ret;
}

bool FolderSync::RetrieveFolder(const std::string& post_id, Folder& out) {
    bool ret = false;
    // Check Post map first
    pm_mtx_.Lock();
    PostMap::iterator itr = post_map_.find(post_id);
    if(itr != post_map_.end()) {
        out = itr->second.folder();
        ret = true;
    }
    pm_mtx_.Unlock();
    // Check Local Cache
    if(!ret)
        ret = file_manager_->GetFolderEntryByPostId(post_id, out);
    // Retrieve FolderPost
    if(!ret) {
        FolderHandler fh(file_manager_);
        FolderPost fp;
        if(fh.RetrieveFolderPost(post_id, 
                                 entity_url_,
                                 post_path_,
                                 at_,
                                 fp) == ret::A_OK) {
            ret = fh.InsertFolder(fp);
            out = fp.folder();
        }
    }
    return ret;
}

}// namespace

