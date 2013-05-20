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
    std::cout<<" validating folder post " << std::endl;
    std::string full_filepath;
    file_manager_->GetCanonicalFilepath(fp.folder().folderpath(), full_filepath);
    bool ret = true;
    if(!full_filepath.empty()) {
        // Check if folder exists, if not, create it
        if(!fs::CheckFilepathExists(full_filepath)) 
            fs::CreateDirectoryTree(full_filepath);

        // Check if there is a corresponding folder entry
        Folder folder;
        if(!file_manager_->GetFolderEntry(fp.folder().folderpath(), folder)) {
            ret = file_manager_->CreateFolderEntry(fp.folder().folderpath(), 
                                                   fp.id(), 
                                                   fp.folder().parent_post_id(), 
                                                   folder);
        }
        else {
            if(folder.folder_post_id().empty())
                ret = file_manager_->SetFolderPostId(fp.folder().folderpath(), fp.id());
        }
    }
    return ret;
}

// Pass in an absolute folderpath
//  - Creates an entry in the local cache
//  - Creates folder on disk if one doesn't already exist
bool FolderHandler::CreateFolder(const std::string& folderpath, std::deque<Folder>& out) {
    bool ret = false;
    if(!folderpath.empty()) { 
        std::deque<std::string> folder_list;
        fs::CreateDirectoryTree(folderpath, file_manager_->working_directory(), folder_list);

        std::cout<<" # of folders " << folder_list.size() << std::endl;
        std::deque<std::string>::iterator itr = folder_list.begin();
        for(;itr!= folder_list.end(); itr++) {
            if(!fs::CheckFilepathExists(*itr)) {
                // Check for local folder, create if doesn't exist
                fs::CreateDirectory(*itr);
            }    
            Folder tmp;
            if(!file_manager_->GetFolderEntry(*itr, tmp)) { 
                // FileManager Entry
                std::cout<< " Creating folder entry ... for " << *itr << std::endl;
                Folder folder;
                ret = file_manager_->CreateFolderEntry(*itr, "", "", folder);
                if(ret)
                    out.push_back(folder);
            }
            else {
                out.push_back(tmp);
                ret = true;
            }
        }
    }
    return ret;
}

bool FolderHandler::SetFolderPostId(Folder& folder, const std::string& post_id) {
    return file_manager_->SetFolderPostId(folder.folderpath(), post_id);
}

bool FolderHandler::SetFolderParentPostId(Folder& folder, const std::string& post_id) {
    return file_manager_->SetFolderParentPostId(folder.folderpath(), post_id);
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
        RetrieveAllFilesAndFoldersInFolder(folder, file_out, folder_out);
        // mark all as deleted
        std::deque<Folder>::iterator itr = folder_out.begin();
        for(;itr!= folder_out.end(); itr++) {
            file_manager_->MarkFilesInFolderDeleted(*itr);
        }
    }
}

void FolderHandler::RenameFolder(const std::string& old_folderpath, 
                                 const std::string& new_folderpath) {
    // Retrieve all sub folders and files
    std::deque<FileInfo> file_list;
    std::deque<Folder> folder_list;
    Folder folder;
    if(file_manager_->GetFolderEntry(old_folderpath, folder)){
        RetrieveAllFilesAndFoldersInFolder(folder, file_list, folder_list);
        std::string aliased_old_path, aliased_new_path;
        if(file_manager_->GetAliasedFilepath(old_folderpath, aliased_old_path) &&
           file_manager_->GetAliasedFilepath(new_folderpath, aliased_new_path)) {
            RenameHandler rh(file_manager_);
            // Update folderpaths
            std::deque<Folder>::iterator folder_itr = folder_list.begin();
            for(;folder_itr!=folder_list.end(); folder_itr++) {
                size_t pos = (*folder_itr).folderpath().find(aliased_old_path);
                if(pos != std::string::npos) {
                    std::string path = aliased_new_path;
                    path += "/";
                    path += (*folder_itr).folderpath().substr(pos);
                    std::cout<<" NEW FOLDER PATH " << path << std::endl;
                    rh.RenameFolderLocalCache((*folder_itr).folderpath(), path);
                    (*folder_itr).set_folderpath(path);
                }
            }
            // Update filepath
            std::deque<FileInfo>::iterator file_itr = file_list.begin();
            for(;file_itr!= file_list.end(); file_itr++) {
                size_t pos = (*file_itr).filepath().find(aliased_old_path);
                if(pos != std::string::npos) {
                    std::string path = aliased_new_path;
                    path += "/";
                    path += (*file_itr).filepath().substr(pos);
                    std::cout<<" NEW FILE PATH " << path << std::endl;
                    rh.RenameFileLocalCache((*file_itr).filepath(), path);
                    (*file_itr).set_filepath(path);
                }
            }
        }
    }
}

int FolderHandler::RetrieveAllFilesAndFoldersInFolder(Folder& folder, 
                                                      std::deque<FileInfo>& file_out,
                                                      std::deque<Folder>& folder_out) {
    int status = ret::A_OK;
    // Retrieve all folders
    status = RetrieveSubFolders(folder, folder_out);
    // retrieve all file ids
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
    while(hold.size()) {
        std::string folder_id = hold.front().folder_post_id();
        out.push_back(hold.front());
        hold.pop_front();
        status = file_manager_->GetAllFoldersForFolder(folder_id, hold);
    }

    return status;
}

int FolderHandler::RetrieveFilesInFolder(Folder& folder, std::deque<FileInfo>& out) {
    return file_manager_->GetAllFileInfoForFolder(folder.folder_post_id(), out);
}



void MarkFolderDeleted(FolderPost& fp) {
    fp.set_fragment(cnst::g_deleted_fragment);
}

}//namespace

