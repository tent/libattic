#include "folderhandler.h"

#include "filesystem.h"
#include "constants.h"

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
                std::cout<< " Creating folder entry ... " << std::endl;
                Folder folder;
                ret = file_manager_->CreateFolderEntry(*itr, "", "", folder);
                if(ret)
                    out.push_back(folder);
            }
            else {
                out.push_back(tmp);
            }
        }
    }
    return ret;
}

void FolderHandler::SetFolderPostId(Folder& folder, const std::string& post_id) {
    file_manager_->SetFolderPostId(folder.folderpath(), post_id);
}

void FolderHandler::SetFolderParentPostId(Folder& folder, const std::string& post_id) {
    file_manager_->SetFolderParentPostId(folder.folderpath(), post_id);
}

// Pass in full path
// - deletes folder from drive
// - removes folder from local cache
// - marks folder post as deleted (fragment)
// - deletes all content
// - marks all content posts as deleted (fragment)
void FolderHandler::DeleteFolder(const std::string& folderpath) {
    Folder folder;
    if(file_manager_->GetFolderEntry(folderpath, folder)){

    }
    //  - purge all records of files/folders within, update posts
    //  - remove from folder table
    // Check for local folder
    if(fs::CheckFilepathExists(folderpath)) {
        //  - delete if still exists
    }
}

void FolderHandler::RenameFolder(const std::string& old_folderpath, 
                                 const std::string& new_folderpath) {
    // Check for local folder
    // - perform rename
    // - update local cache
    // - update post
    // - update all contents
}

void MarkFolderDeleted(FolderPost& fp) {
    fp.set_fragment(cnst::g_deleted_fragment);
}

}//namespace

