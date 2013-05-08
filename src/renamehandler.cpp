#include "renamehandler.h"

#include <iostream>

#include "netlib.h"
#include "filemanager.h"
#include "filesystem.h"

namespace attic { 

RenameHandler::RenameHandler(FileManager* fi) {
    file_manager_ = fi;
}

RenameHandler::~RenameHandler() {
    file_manager_ = NULL;
}

int RenameHandler::RenameFileLocalCache(const std::string& old_filepath, 
                                        const std::string& new_name,
                                        std::string& new_filepath) {
    int status = ret::A_OK;
    FileInfo* fi = file_manager_->GetFileInfo(old_filepath);
    if(fi) {
        status = file_manager_->RenameFile(fi->filepath(), 
                                           new_name, 
                                           new_filepath);
        std::cout<<" out filepath : " << new_filepath << std::endl;
    }
    return status;
}

void RenameHandler::UpdateFileMetaPost(FilePost& fp, 
                                       const FileInfo& fi, 
                                       FilePost& out) {
    out = fp;
    out.PushBackAlias(fp.relative_path()); // Push back old path
    out.set_relative_path(fi.filepath());
    out.set_name(fi.filename());

    Parent parent;
    parent.version = fp.version()->id();
    out.PushBackParent(parent);
}

bool RenameHandler::CheckForRename(FilePost& fp) {
    std::cout<<" CHECKING FOR RENAME " << std::endl;
    FileInfo* fi = file_manager_->GetFileInfoByPostId(fp.id());
    if(fi) {
        // If filepaths are the same
        if(fi->filepath() != fp.relative_path()){
            std::cout<< " FILEPATHS DIFFER, RENAME " << std::endl;
            // Update Local cache
            std::string old_filepath = fi->filepath();
            std::string new_filepath;
            int status = RenameFileLocalCache(fi->filepath(), fp.name(), new_filepath);
            std::cout<<" new filepath : " << new_filepath << std::endl;
            if(status == ret::A_OK) {
                // Rename physical file
                std::string canonical_old, canonical_new;
                file_manager_->GetCanonicalFilepath(old_filepath, canonical_old);
                file_manager_->GetCanonicalFilepath(new_filepath, canonical_new);
                std::cout<<" OLD : " << canonical_old << std::endl;
                std::cout<<" NEW : " << canonical_new << std::endl;
                if(!canonical_new.empty()) {
                    if(fs::CheckFilepathExists(canonical_old)){
                        std::cout<<" RENAMING " << std::endl;
                        fs::RenamePath(canonical_old, canonical_new);
                        return true;
                    }
                    else {
                        std::cout<<" OLD PATH DOESNT EXIST " << std::endl;
                    }
                }
                else {
                    std::cout<<" CANONICAL NEW IS EMPTY : " << canonical_new << std::endl;
                }
            }
            else {
                std::cout<<" RENAME LOCAL CACHE FAILED " << std::endl;
            }
        }
    }
    std::cout << " FAILED TO RENAME " << std::endl;
    return false;
}

}//namespace
