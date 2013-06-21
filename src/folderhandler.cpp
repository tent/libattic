#include "folderhandler.h"

#include "filesystem.h"
#include "constants.h"
#include "renamehandler.h"
#include "logutils.h"

#include "posthandler.h"
#include "foldercreationlock.h"

namespace attic {
FolderHandler::FolderHandler(FileManager* fm) {
    file_manager_ = fm;
}

FolderHandler::~FolderHandler() {}

bool FolderHandler::ValidateFolder(FolderPost& fp) {
    bool ret = false;
    std::cout<<" validating folder post " << std::endl;
    // check if parent exists
    // check if folder entry exists
    //  if so check for rename
    //  if not create entry
    //   create folder
    std::cout<<" parent folder post id : " << fp.folder().parent_post_id() << std::endl;
    if(file_manager_->DoesFolderExistById(fp.folder().parent_post_id())) {
        std::string folderpath, full_folderpath;
        if(file_manager_->ConstructFolderpath(fp.id(), folderpath)) {
            std::cout<<" folder path : " << folderpath << std::endl;
            if(file_manager_->GetCanonicalPath(folderpath, full_folderpath)) { 
                std::cout<<" creating directory tree for " << full_folderpath << std::endl;
            }
        }
        if(full_folderpath.empty()) {
            std::ostringstream error;
            error << "Validate Folder, full folderpath empty ";
            error << " post id : " << fp.id() << std::endl;
            error << " foldername : " << fp.folder().foldername() << std::endl;
            log::LogString("folder_handler_12904", error.str());
            return ret;
        }

        if(file_manager_->DoesFolderExistById(fp.folder().folder_post_id()) && 
                                  fs::CheckFilepathExists(full_folderpath)) {
            // check for rename
            std::cout<<" CHECK FOR RENAME " << std::endl;
            RenameHandler rh(file_manager_);
            rh.CheckForRename(fp);
        }
        else {
            std::cout<<" creating folder entry " << std::endl;
            //create entry
            if(InsertFolder(fp)) {
                std::cout<<" attempting to constuct folderpath " << std::endl;
                std::cout<<" folder id : " << fp.id() << std::endl;
                std::string folderpath;
                if(file_manager_->ConstructFolderpath(fp.id(), folderpath)) {
                    std::cout<<" folder path : " << folderpath << std::endl;
                    std::string full_folderpath;
                    try {
                        //create folder
                        fs::CreateDirectoryTreeForFolder(full_folderpath);
                        ret = true;
                    }
                    catch(std::exception& e) {
                        log::LogException("fh_1281jn1", e);
                    }
                }
                else {
                    std::cout<<" failed to create folderpath " <<std::endl;
                }
            }
        }
    }

    return ret;
}


bool FolderHandler::ValidateFolderPath(const std::string& folderpath, 
                                       const std::string& posts_feed,
                                       const std::string& post_path,
                                       const AccessToken& at) {
    bool ret = true;
    std::ostringstream vlog;
    vlog <<" **************************************************** " << std::endl;
    vlog <<" VALIDATING FOLDER PATH : " << folderpath << std::endl;
    vlog <<" posts feed : " << posts_feed << std::endl;
    vlog <<" post path : " << post_path << std::endl;
    vlog <<" folder path : " << folderpath << std::endl;
    // absolute filepath
    if(!folderpath.empty()) {
        std::string directory, directory_post_id;
        if(file_manager_->FindAssociatedWorkingDirectory(folderpath, 
                                                         directory, 
                                                         directory_post_id)) {
            vlog << " working directory : " << directory << std::endl;
            vlog << " directory post id : " << directory_post_id << std::endl;
            std::deque<std::string> names;
            if(RetrieveFolders(folderpath, directory, names)) {
                vlog << " retrieved folders " << std::endl;
                std::deque<std::string>::iterator n_itr = names.begin();
                for(;n_itr!=names.end();n_itr++) {
                    vlog <<" "<< (*n_itr) << std::endl;
                }
                // validate each folder exists
                fcl l; // File lock
                std::deque<std::string>::iterator itr = names.begin();
                std::string parent_post_id = directory_post_id;
                for(;itr!=names.end();itr++) {
                    l.TryLock((*itr), parent_post_id);
                    Folder folder;
                    bool exists = file_manager_->GetFolderEntry((*itr), parent_post_id, folder);
                    vlog <<" folder : " << (*itr) << " id : " << parent_post_id << std::endl;
                    vlog << " exists : " << exists << std::endl;
                    if(!exists) {
                        vlog << " creating folder " << std::endl;
                        vlog << " name : " << (*itr) << std::endl;
                        vlog << " parent post id " << parent_post_id << std::endl;
                        folder.set_foldername(*itr);
                        folder.set_parent_post_id(parent_post_id);
                        //  if not create post
                        std::string post_id;
                        if(CreateFolderPost(folder, posts_feed, at, post_id) == ret::A_OK) {
                            folder.set_folder_post_id(post_id);
                            file_manager_->CreateFolderEntry(folder.foldername(),
                                                             folder.folder_post_id(),
                                                             folder.parent_post_id(),
                                                             folder);
                        }
                        else {
                            vlog << " failed to create folder post breaking " << std::endl;
                            ret = false;
                            l.Unlock((*itr), parent_post_id);
                            break;
                        }
                    }
                    else {
                        vlog << " file already exists " << std::endl;
                        // Check if folderpath is deleted
                        if(file_manager_->IsFolderDeleted(folder.folder_post_id())){
                            // Un-delete
                            file_manager_->SetFolderDeleted(folder.folder_post_id(), false);
                            UpdateFolderPost(folder, 
                                             folder.folder_post_id(),
                                             post_path,
                                             at);
                        }
                    }
                    l.Unlock((*itr), parent_post_id);
                    parent_post_id = folder.folder_post_id();
                    vlog << " setting parent post id : " << parent_post_id << std::endl;

                } // for
            }
            else {
                vlog << " failed to retreive folders " << std::endl;
                std::cout<<" failed to retreive folders " << std::endl;
                ret = false;
            }
        }
    }
    else {
        vlog << " folderpath empty " << std::endl;
        ret = false;
    }

    vlog <<" **************************************************** " << std::endl;
    std::cout<< vlog.str() << std::endl;

    return ret;
}


// Pass in an absolute folderpath
// Pass in the associated working directory
// creates a list of folder entries
bool FolderHandler::RetrieveFolders(const std::string& folderpath, 
                                    const std::string& working_directory,
                                    std::deque<std::string>& out) {
    bool ret = false;
    std::string path = folderpath;
    size_t pos = path.find(working_directory);
    if(pos != std::string::npos) {
        std::string cat_path = path.substr(pos+working_directory.size());
        utils::SeparatePath(cat_path, out);
        ret = true;
    }
    return ret;
}

bool FolderHandler::InsertFolder(const Folder& folder) {
    bool ret = false;
    if(!IsFolderInCacheWithId(folder.folder_post_id())) {
        if(!IsFolderInCache(folder.foldername(), folder.parent_post_id())) {
            Folder f;
            ret = file_manager_->CreateFolderEntry(folder.foldername(),
                                                   folder.folder_post_id(),
                                                   folder.parent_post_id(),
                                                   f);
        }
        else {
            std::ostringstream err;
            err << " Attempting to Insert duplicate folder (same name diff post) " << std::endl;
            log::LogString("fh_19485", err.str());
        }
    }
    return ret;
}

bool FolderHandler::InsertFolder(const FolderPost& fp) {
    bool ret = false;
    if(!IsFolderInCacheWithId(fp.id())) {
        if(!IsFolderInCache(fp.folder().foldername(), fp.folder().parent_post_id())) {
            Folder f;
            ret = file_manager_->CreateFolderEntry(fp.folder().foldername(),
                                                   fp.id(),
                                                   fp.folder().parent_post_id(),
                                                   f);
        }
        else {
            std::ostringstream err;
            err << " Attempting to Insert duplicate folder (same name diff post) " << std::endl;
            log::LogString("fh_19485", err.str());
        }
    }

    return ret;
}

bool FolderHandler::SetFolderPostId(Folder& folder, const std::string& post_id) {
    return file_manager_->SetFolderPostId(folder.foldername(), post_id);
}

bool FolderHandler::SetFolderParentPostId(Folder& folder, const std::string& post_id) {
    folder.set_parent_post_id(post_id);
    return file_manager_->SetFolderParentPostId(folder.foldername(), post_id);
}

void FolderHandler::DeleteFolder(const std::string& folderpath) {
    std::deque<FileInfo> file_list;
    std::deque<Folder> folder_list;
    DeleteFolder(folderpath, file_list, folder_list);
}

// Pass in full path
// - deletes folder from drive
// - removes folder from local cache
// - marks folder post as deleted (fragment)
// - deletes all content
// - marks all content posts as deleted (fragment)
void FolderHandler::DeleteFolder(const std::string& folderpath, 
                                 std::deque<FileInfo>& file_out,
                                 std::deque<Folder>& folder_out) {

    Folder folder;
    if(file_manager_->GetFolderEntry(folderpath, folder)){
        file_manager_->SetFolderDeleted(folderpath, true);
        std::cout<<" retrieving all files and folders in folder : " << folderpath << std::endl;
        RetrieveAllFilesAndFoldersInFolder(folder, file_out, folder_out);
        std::cout<< " file count : " << file_out.size() << std::endl;
        std::cout<< " folder count : " << folder_out.size() << std::endl;
        // mark all as deleted
        std::deque<Folder>::iterator itr = folder_out.begin();
        for(;itr!= folder_out.end(); itr++) {
            file_manager_->SetFolderDeleted((*itr).foldername(), true);
            file_manager_->MarkFilesInFolderDeleted(*itr);
        }
    }
}

void FolderHandler::RenameFolder(const std::string& old_folderpath, 
                                 const std::string& new_folderpath,
                                 std::deque<FileInfo>& file_list,
                                 std::deque<Folder>& folder_list) {
    std::ostringstream rlog;
    rlog <<" **************************************************** " << std::endl;
    rlog <<" rename folder called " << std::endl;
    // Retrieve all sub folders and files
    Folder folder;
    if(file_manager_->GetFolderEntry(old_folderpath, folder)){
        RetrieveAllFilesAndFoldersInFolder(folder, file_list, folder_list);
        rlog << " folder list : " << folder_list.size() << std::endl;
        rlog << " file list : " << file_list.size() << std::endl;
        std::string aliased_old_path, aliased_new_path;
        if(file_manager_->GetAliasedPath(old_folderpath, aliased_old_path) &&
           file_manager_->GetAliasedPath(new_folderpath, aliased_new_path)) {
            rlog <<" aliased folderpath (old): " << aliased_old_path << std::endl;
            rlog <<" aliased folderpath (new): " << aliased_new_path << std::endl;
            RenameHandler rh(file_manager_);
            // Update folderpaths
            std::deque<Folder>::iterator folder_itr = folder_list.begin();
            for(;folder_itr!=folder_list.end(); folder_itr++) {
                std::cout<< (*folder_itr).foldername() << std::endl;
                size_t pos = (*folder_itr).foldername().find(aliased_old_path);
                if(pos != std::string::npos) {
                    std::string path = aliased_new_path;
                    path += (*folder_itr).foldername().substr(pos+aliased_old_path.size());
                    size_t f = path.find("//");
                    if(f!= std::string::npos)
                        path.erase(f, 1);
                    rlog <<" NEW FOLDER PATH " << path << std::endl;
                    // Normalize path
                    utils::CheckUrlAndRemoveTrailingSlash(path);
                    rh.RenameFolderLocalCache((*folder_itr).foldername(), path);
                    //(*folder_itr).set_foldername(aliased_new_path);
                    (*folder_itr).set_foldername(path);
                    // Update folder post
                    size_t p_pos = aliased_new_path.rfind("/");
                    if(p_pos != std::string::npos) {
                        std::string parent_path = aliased_new_path.substr(0, p_pos);
                        // Normalize path
                        utils::CheckUrlAndRemoveTrailingSlash(parent_path);
                        Folder parent_folder;
                        rlog <<" parent path : " << parent_path << std::endl;
                        if(GetFolder(parent_path, parent_folder)) {
                            rlog << " current folder : " << (*folder_itr).foldername() << std::endl;
                            rlog << " setting new parent id " << std::endl;
                            rlog << parent_folder.folder_post_id() << std::endl;
                            (*folder_itr).set_parent_post_id(parent_folder.folder_post_id());
                            SetFolderParentPostId((*folder_itr), parent_folder.folder_post_id());
                        }
                        else {
                            std::cout<<" folder post doesn't exist " << std::endl;
                        }
                    }
                    else {
                        std::cout<<" end of the line ... " << std::endl;
                    }
                }
            }
            // Update filepath
            std::deque<FileInfo>::iterator file_itr = file_list.begin();
            for(;file_itr!= file_list.end(); file_itr++) {
                rlog <<" finding old path : " << aliased_old_path << std::endl;
                size_t pos = (*file_itr).filepath().find(aliased_old_path);
                if(pos != std::string::npos) {
                    std::string path = aliased_new_path;
                    utils::CheckUrlAndRemoveTrailingSlash(path);
                    path += (*file_itr).filepath().substr(pos+aliased_old_path.size());
                    rlog <<" NEW FILE PATH " << path << std::endl;
                    rlog <<" aliased new path : " << aliased_new_path << std::endl;

                    rh.RenameFileLocalCache((*file_itr).filepath(), path);
                    (*file_itr).set_filepath(path);
                }
            }
        }
    }
    else {
        rlog <<" could not get folder entry " << std::endl;
    }

    rlog <<" **************************************************** " << std::endl;
    std::cout<< rlog.str() << std::endl;
}


int FolderHandler::RetrieveAllFilesAndFoldersInFolder(Folder& folder, 
                                                      std::deque<FileInfo>& file_out,
                                                      std::deque<Folder>& folder_out) {
    int status = ret::A_OK;
    // Retrieve all folders
    std::cout<<" retrieving sub folders " << std::endl;
    status = RetrieveSubFolders(folder, folder_out);
    // retrieve all file ids
    std::cout<<" retrieving files " << std::endl;
    std::deque<Folder>::iterator itr = folder_out.begin();
    for(;itr!= folder_out.end();itr++){
        RetrieveFilesInFolder(*itr, file_out);
    }
    return status;
}

int FolderHandler::RetrieveSubFolders(Folder& folder, std::deque<Folder>& out) {
    int status = ret::A_OK;
    std::deque<Folder> hold;
    hold.push_back(folder);
    std::string last_id;
    while(hold.size()) {
        std::string folder_id = hold.front().folder_post_id();
        out.push_back(hold.front());
        hold.pop_front();
        if(last_id != folder_id) {
            std::cout<<" get all folders for : " << folder_id << std::endl;
            status = file_manager_->GetAllFoldersForFolder(folder_id, hold);
            last_id = folder_id;
        }
    }

    return status;
}

int FolderHandler::RetrieveFilesInFolder(Folder& folder, std::deque<FileInfo>& out) {
    return file_manager_->GetAllFileInfoForFolder(folder.folder_post_id(), out);
}

void FolderHandler::MarkFolderDeleted(FolderPost& fp) {
    fp.set_fragment(cnst::g_deleted_fragment);
}

bool FolderHandler::GetFolderById(const std::string& folder_id, Folder& out) {
    return file_manager_->GetFolderEntryByPostId(folder_id, out);
}

bool FolderHandler::GetFolder(const std::string& folderpath, Folder& out) {
    return file_manager_->GetFolderEntry(folderpath, out);
}

bool FolderHandler::IsFolderInCache(const std::string& folder_name, const std::string& parent_post_id) {
    return file_manager_->DoesFolderExist(folder_name, parent_post_id);
}

bool FolderHandler::IsFolderInCacheWithId(const std::string& post_id) {
    return file_manager_->DoesFolderExistById(post_id);
}

bool FolderHandler::SetFolderDeleted(const std::string& folderpath, bool del) {
    return file_manager_->SetFolderDeleted(folderpath, del);
}


int FolderHandler::CreateFolderPost(Folder& folder, 
                                    const std::string& posts_feed,
                                    const AccessToken& at,
                                    std::string& id_out) {
    int status = ret::A_OK;
    // Create folderpost
    FolderPost fp(folder);
    PostHandler<FolderPost> ph(at);
    status = ph.Post(posts_feed, NULL, fp);
    if(status == ret::A_OK) {
        FolderPost back = ph.GetReturnPost();
        folder.set_folder_post_id(back.id());
        id_out = back.id();
    }
    else {
        log::LogHttpResponse("fh_u19845", ph.response());
    }
    return status;
}

bool FolderHandler::UpdateFolderPost(Folder& folder, 
                                     const std::string& post_id,
                                     const std::string& post_path,
                                     const AccessToken& at) {
    bool ret = false;
    if(!post_id.empty()) {
        FolderPost fp;
        if(RetrieveFolderPost(folder.folder_post_id(), post_path, at, fp) == ret::A_OK) {
            fp.set_folder(folder);
            if(PostFolderPost(post_id, post_path, at, fp) == ret::A_OK)
                ret = true;
        }
    }
    else {
        std::ostringstream err;
        err << " Empty Folder Post id : " << std::endl;
        err << " entry : " << folder.foldername();
        err << " \t\t " << folder.folder_post_id();
        err << " \t\t " << folder.parent_post_id();
        log::LogString("fh_9_18912512", err.str());
    }
    return ret;
}

int FolderHandler::RetrieveFolderPost(const std::string& post_id, 
                                      const std::string& post_path,
                                      const AccessToken& at,
                                      FolderPost& out) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string pp = post_path;
        std::string posturl;
        utils::FindAndReplace(pp, "{post}", post_id, posturl);
        PostHandler<FolderPost> ph(at);
        status = ph.Get(posturl, NULL, out);
        if(status != ret::A_OK)
            log::LogHttpResponse("fh_9_13581", ph.response());
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

int FolderHandler::PostFolderPost(const std::string& post_id, 
                                  const std::string& post_path,
                                  const AccessToken& at,
                                  FolderPost& fp) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string pp = post_path;
        std::string posturl;
        utils::FindAndReplace(pp, "{post}", post_id, posturl);
        PostHandler<FolderPost> ph(at);
        status = ph.Put(posturl, NULL, fp);
        if(status != ret::A_OK)
            log::LogHttpResponse("fh_85_n342309", ph.response());
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

}//namespace

