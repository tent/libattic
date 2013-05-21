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
                                        const std::string& new_filepath) {
    int status = ret::A_OK;
    status = file_manager_->RenameFile(old_filepath,
                                       new_filepath);
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

int RenameHandler::RenameFolderLocalCache(const std::string& old_folderpath,
                                          const std::string& new_folderpath) {
    int status = ret::A_OK;
    std::cout<<" rename folder local cache " << std::endl;
    status = file_manager_->RenameFolder(old_folderpath, new_folderpath);
    return status;
}

void RenameHandler::UpdateFolderMetaPost(FolderPost& fp,
                                         const Folder& folder,
                                         FolderPost& out) {
    out = fp;
    Folder f = folder;
    f.PushBackAlias(fp.folder().folderpath());
    out.set_folder(f);

    Parent parent;
    parent.version = fp.version()->id();
    out.PushBackParent(parent);
}

bool RenameHandler::CheckForRename(FolderPost& fp) {
    std::cout<<" Checking for folder RENAME " << std::endl;
    Folder folder;
    std::cout<<" checking for folder with id : " << fp.id() << std::endl;
    if(file_manager_->GetFolderEntryByPostId(fp.id(), folder)) {
        if(folder.folderpath() != fp.folder().folderpath()) {
            // Update local cache
            std::string old_folderpath = folder.folderpath();
            std::string new_folderpath = fp.folder().folderpath();

            int status = RenameFolderLocalCache(old_folderpath, new_folderpath);
            if(status = ret::A_OK) {
                // Rename physical file
                std::string canonical_old, canonical_new;
                file_manager_->GetCanonicalFilepath(old_folderpath, canonical_old);
                file_manager_->GetCanonicalFilepath(new_folderpath, canonical_new);
                std::cout<<" OLD : " << canonical_old << std::endl;
                std::cout<<" NEW : " << canonical_new << std::endl;
                if(!canonical_new.empty()) {
                    if(fs::CheckFilepathExists(canonical_old)) {
                        std::cout<<" RENAMING FOLDER " << std::endl;
                        fs::RenamePath(canonical_old, canonical_new);
                        return true;
                    }
                }
            }
        }
    }
    return false;
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
            std::string new_filepath = fp.relative_path();

            std::cout << "local filepath : " << fi->filepath() << std::endl;
            std::cout << "post filepath : " << fp.relative_path() << std::endl;
            int status = RenameFileLocalCache(old_filepath, new_filepath);
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

