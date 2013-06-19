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
    // rename file (filepath and filename)
    status = file_manager_->RenameFile(old_filepath,
                                       new_filepath);
    // update folder post id
    return status;
}

void RenameHandler::UpdateFileMetaPost(FilePost& fp, 
                                       const FileInfo& fi, 
                                       FilePost& out) {

    out = fp;
    out.set_file_info(fi);
    Parent parent;
    parent.version = fp.version().id();
    out.PushBackParent(parent);
}

int RenameHandler::RenameFolderLocalCache(const std::string& old_folderpath,
                                          const std::string& new_folderpath) {
    int status = ret::A_OK;
    std::cout<<" rename folder local cache " << std::endl;
    std::string ofp = old_folderpath;
    std::string nfp = new_folderpath;
    // Normalize paths
    utils::CheckUrlAndRemoveTrailingSlash(ofp);
    utils::CheckUrlAndRemoveTrailingSlash(nfp);
    std::cout<<" \t old : " << ofp << std::endl;
    std::cout<<" \t new : " << nfp << std::endl;
    status = file_manager_->RenameFolder(ofp, nfp);
    return status;
}

void RenameHandler::UpdateFolderMetaPost(FolderPost& fp,
                                         const Folder& folder,
                                         FolderPost& out) {
    out = fp;
    Folder f = folder;
    out.set_folder(f);

    Parent parent;
    parent.version = fp.version().id();
    out.PushBackParent(parent);
}

bool RenameHandler::CheckForRename(FolderPost& fp) {
    bool ret = false;
    // Check if folder post is different
    // name change
    // parent_post_id change
    // folderpath change (check parent_post_id) 
    std::cout<<" CHECK FOR fOLDER RENAME : " << std::endl;
    Folder folder;
    if(file_manager_->GetFolderEntryByPostId(fp.id(), folder)) {
        std::cout<<" FOLDER EXISTS : " << fp.folder().foldername() << std::endl;
        if(folder.foldername() != fp.folder().foldername()) {
            std::cout<<" RENAMEING : " << folder.foldername() << std::endl;
            std::cout<<" TO : " << fp.folder().foldername() << std::endl;
            // Construct old path
            std::string old_path;
            file_manager_->ConstructFolderpath(folder.folder_post_id(), old_path);
            ret = file_manager_->SetFoldername(folder.folder_post_id(), 
                                               fp.folder().foldername());
            // Constrcut new path
            std::string new_path;
            file_manager_->ConstructFolderpath(folder.folder_post_id(), new_path);
            // Get Absoulte paths
            // Rename
            std::cout<<" old path : " << old_path << std::endl;
            std::cout<<" new path : " << new_path << std::endl;

            std::string old_can, new_can;
            file_manager_->GetCanonicalPath(old_path, old_can);
            file_manager_->GetCanonicalPath(new_path, new_can);
            std::cout<<" old canonical : " << old_can << std::endl;
            std::cout<<" new canonical : " << new_can << std::endl;
            try {
                fs::RenamePath(old_can, new_can);
            }
            catch(std::exception& e) {
                std::cout<<" EXCEPTION WHAT : " << e.what() << std::endl;
            }
        }

        if(folder.parent_post_id() != fp.folder().parent_post_id()) {
            std::cout<<" PARENTS DIFFER " << std::endl;
            // Construct old path
            std::string old_path;
            file_manager_->ConstructFolderpath(folder.folder_post_id(), old_path);
            ret = file_manager_->SetFolderParentPostId(folder.folder_post_id(),
                                                        fp.folder().parent_post_id());
            // Constrcut new path
            // move to new folder
            // Constrcut new path
            std::string new_path;
            file_manager_->ConstructFolderpath(folder.folder_post_id(), new_path);
            // Rename
            std::cout<<" old path : " << old_path << std::endl;
            std::cout<<" new path : " << new_path << std::endl;

            std::string old_can, new_can;
            file_manager_->GetCanonicalPath(old_path, old_can);
            file_manager_->GetCanonicalPath(new_path, new_can);
            std::cout<<" old canonical : " << old_can << std::endl;
            std::cout<<" new canonical : " << new_can << std::endl;
            try {
                fs::RenamePath(old_can, new_can);
            }
            catch(std::exception& e) {
                std::cout<<" EXCEPTION WHAT : " << e.what() << std::endl;
            }
        }

    }
    return ret;
}

// Checks FileInfo (preferably generated from a file post against the local cache)
bool RenameHandler::CheckForRename(FileInfo& fi, const std::string& post_id) {
    std::cout<<" CHECKING FOR RENAME " << std::endl;
    std::cout<<" Incoming filepath : " << fi.filepath() << std::endl;
    FileInfo local_fi;
    if(file_manager_->GetFileInfoByPostId(post_id, local_fi)) {
        // If filepaths are the same
        if(local_fi.filepath() != fi.filepath()){
            std::cout<< " FILEPATHS DIFFER, RENAME " << std::endl;
            // Update Local cache
            std::string old_filepath = local_fi.filepath();
            std::string new_filepath = fi.filepath();

            std::cout << "local filepath : " << local_fi.filepath() << std::endl;
            std::cout << "post filepath : " << fi.filepath() << std::endl;
            int status = RenameFileLocalCache(old_filepath, new_filepath);
            std::cout<<" new filepath : " << new_filepath << std::endl;
            if(status == ret::A_OK) {
                // Rename physical file
                std::string canonical_old, canonical_new;
                file_manager_->GetCanonicalPath(old_filepath, canonical_old);
                file_manager_->GetCanonicalPath(new_filepath, canonical_new);
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
    return false;
}

}//namespace

