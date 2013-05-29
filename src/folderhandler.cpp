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

    RenameHandler rh(file_manager_);
    file_manager_->GetCanonicalFilepath(fp.folder().folderpath(), full_filepath);
    bool ret = true;

    std::cout<<"checking for filepath : " << full_filepath << std::endl;
    if(!full_filepath.empty()) {
        if(!rh.CheckForRename(fp)) { 
            // Check if folder exists, if not, create it
            if(!fs::CheckFilepathExists(full_filepath)) {
                std::cout<<" creating directory tree for " << full_filepath << std::endl;
                try {
                    fs::CreateDirectoryTreeForFolder(full_filepath);
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
        std::cout<<" working directory : " << std::endl;

        if(!folder_list.size())
            folder_list.push_back(folderpath);

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
            file_manager_->SetFolderDeleted((*itr).folderpath(), true);
            file_manager_->MarkFilesInFolderDeleted(*itr);
        }
    }
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
        if(file_manager_->GetAliasedFilepath(old_folderpath, aliased_old_path) &&
           file_manager_->GetAliasedFilepath(new_folderpath, aliased_new_path)) {
            std::cout<<" aliased folderpath (old): " << aliased_old_path << std::endl;
            std::cout<<" aliased folderpath (new): " << aliased_new_path << std::endl;
            RenameHandler rh(file_manager_);
            // Update folderpaths
            std::deque<Folder>::iterator folder_itr = folder_list.begin();
            for(;folder_itr!=folder_list.end(); folder_itr++) {
                size_t pos = (*folder_itr).folderpath().find(aliased_old_path);
                if(pos != std::string::npos) {
                    std::string path = aliased_new_path;
                    utils::CheckUrlAndAppendTrailingSlash(path);
                    path += (*folder_itr).folderpath().substr(pos+aliased_old_path.size());
                    size_t f = path.find("//");
                    if(f!= std::string::npos)
                        path.erase(f, 1);
                    std::cout<<" NEW FOLDER PATH " << path << std::endl;
                    rh.RenameFolderLocalCache((*folder_itr).folderpath(), path);
                    //(*folder_itr).set_folderpath(aliased_new_path);
                    (*folder_itr).set_folderpath(path);
                }
            }
            // Update filepath
            std::deque<FileInfo>::iterator file_itr = file_list.begin();
            for(;file_itr!= file_list.end(); file_itr++) {
                size_t pos = (*file_itr).filepath().find(aliased_old_path);
                if(pos != std::string::npos) {
                    std::string path = aliased_new_path;
                    utils::CheckUrlAndAppendTrailingSlash(path);
                    path += (*file_itr).filepath().substr(pos);
                    std::cout<<" NEW FILE PATH " << path << std::endl;
                    std::cout<<" aliased new path : " << aliased_new_path << std::endl;

                    (*file_itr).PushBackAlias((*file_itr).filepath());
                    rh.RenameFileLocalCache((*file_itr).filepath(), path);
                    (*file_itr).set_filepath(aliased_new_path);
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

