#include "folderhandler.h"

#include "filesystem.h"
#include "constants.h"
#include "renamehandler.h"

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
        if(file_manager_->DoesFolderExistById(fp.folder().folder_post_id())) {
            // check for rename
        }
        else {
            std::cout<<" creating folder entry " << std::endl;
            //create entry
            Folder folder;
            if(file_manager_->CreateFolderEntry(fp.folder().foldername(), 
                                                fp.id(), 
                                                fp.folder().parent_post_id(), 
                                                folder)) {
                std::cout<<" attempting to constuct folderpath " << std::endl;
                std::cout<<" folder id : " << fp.id() << std::endl;
                std::string folderpath;
                if(file_manager_->ConstructFolderpath(fp.id(), folderpath)) {
                    std::cout<<" folder path : " << folderpath << std::endl;
                    std::string full_folderpath;
                    if(file_manager_->GetCanonicalPath(folderpath, full_folderpath)) { 
                        std::cout<<" creating directory tree for " << full_folderpath << std::endl;
                       if(!fs::CheckFilepathExists(full_folderpath)) {
                            try {
                                //create folder
                                fs::CreateDirectoryTreeForFolder(full_folderpath);
                                ret = true;
                            }
                            catch(std::exception& e) {
                                std::cout<<" Caught fs exception : "<< e.what()<< std::endl;
                            }
                       }
                    }
                    else {
                        std::cout<<" renamed ...? " << std::endl;
                    }
                }
                else {
                    std::cout<<" failed to create folderpath " <<std::endl;
                }
            }
        }
    }
/*
    std::string folderpath;
    if(file_manager_->ConstructFolderpath(fp.id(), folderpath)) {
        std::cout<<"checking for folderpath : " << folderpath << std::endl;
        // Get full path
        std::string full_folderpath;
        file_manager_->GetCanonicalPath(folderpath, full_folderpath);
        if(!full_folderpath.empty()) {
            RenameHandler rh(file_manager_);
            if(!rh.CheckForRename(fp)) { 
                // Check if folder exists, if not, create it
                if(!fs::CheckFilepathExists(full_folderpath)) {
                    std::cout<<" creating directory tree for " << full_folderpath << std::endl;
                    try {
                        fs::CreateDirectoryTreeForFolder(full_folderpath);
                    }
                    catch(std::exception& e) {
                        std::cout<<" Caught fs exception : "<< e.what()<< std::endl;
                    }
                }
                else {
                    std::cout<<" renamed ...? " << std::endl;
                }
            }
            // Check if there is a corresponding folder entry
            Folder folder;
            if(!file_manager_->GetFolderEntry(fp.folder().foldername(), folder)) {
                ret = file_manager_->CreateFolderEntry(fp.folder().foldername(), 
                                                       fp.id(), 
                                                       fp.folder().parent_post_id(), 
                                                       folder);
            }
            else {
                if(folder.folder_post_id().empty())
                    ret = file_manager_->SetFolderPostId(fp.folder().foldername(), fp.id());
            }
        }
    }
    */

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
    Folder f;
    return file_manager_->CreateFolderEntry(folder.foldername(),
                                            folder.folder_post_id(),
                                            folder.parent_post_id(),
                                            f);
}

bool FolderHandler::InsertFolder(const FolderPost& fp) {
    Folder f;
    return file_manager_->CreateFolderEntry(fp.folder().foldername(),
                                            fp.id(),
                                            fp.folder().parent_post_id(),
                                            f);
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
    /*
    */
}

void FolderHandler::RenameFolder(const std::string& old_folderpath, 
                                 const std::string& new_folderpath,
                                 std::deque<FileInfo>& file_list,
                                 std::deque<Folder>& folder_list) {
    std::cout<<" rename folder called " << std::endl;
    // Retrieve all sub folders and files
    Folder folder;
    if(file_manager_->GetFolderEntry(old_folderpath, folder)){
        RetrieveAllFilesAndFoldersInFolder(folder, file_list, folder_list);
        std::cout<<" folder list : " << folder_list.size() << std::endl;
        std::cout<<" file list : " << file_list.size() << std::endl;
        std::string aliased_old_path, aliased_new_path;
        if(file_manager_->GetAliasedPath(old_folderpath, aliased_old_path) &&
           file_manager_->GetAliasedPath(new_folderpath, aliased_new_path)) {
            std::cout<<" aliased folderpath (old): " << aliased_old_path << std::endl;
            std::cout<<" aliased folderpath (new): " << aliased_new_path << std::endl;
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
                    std::cout<<" NEW FOLDER PATH " << path << std::endl;
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
                        std::cout<<" parent path : " << parent_path << std::endl;
                        if(GetFolder(parent_path, parent_folder)) {
                            std::cout<< " current folder : " << (*folder_itr).foldername() << std::endl;
                            std::cout<< " setting new parent id " << std::endl;
                            std::cout<< parent_folder.folder_post_id() << std::endl;
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
                std::cout<<" finding old path : " << aliased_old_path << std::endl;
                size_t pos = (*file_itr).filepath().find(aliased_old_path);
                if(pos != std::string::npos) {
                    std::string path = aliased_new_path;
                    utils::CheckUrlAndRemoveTrailingSlash(path);
                    path += (*file_itr).filepath().substr(pos+aliased_old_path.size());
                    std::cout<<" NEW FILE PATH " << path << std::endl;
                    std::cout<<" aliased new path : " << aliased_new_path << std::endl;

                    rh.RenameFileLocalCache((*file_itr).filepath(), path);
                    (*file_itr).set_filepath(path);
                }
            }
        }
    }
    else {
        std::cout<<" could not get folder entry " << std::endl;
    }
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

bool FolderHandler::IsFolderInCache(const std::string& folderpath) {
    return file_manager_->DoesFolderExist(folderpath);
}

bool FolderHandler::SetFolderDeleted(const std::string& folderpath, bool del) {
    return file_manager_->SetFolderDeleted(folderpath, del);
}


}//namespace

