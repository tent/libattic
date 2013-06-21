#include "renamehandler.h"

#include <iostream>

#include "netlib.h"
#include "filemanager.h"
#include "filesystem.h"
#include "logutils.h"

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

bool RenameHandler::RenameFolderLocalCache(const std::string& folder_post_id,
                                          const std::string& new_foldername) {
    std::cout<<" rename folder local cache " << std::endl;
    std::cout<<"\t folder post id : " << folder_post_id << std::endl;
    std::cout<<"\t new foldername : " << new_foldername << std::endl;
    return file_manager_->SetFoldername(folder_post_id, new_foldername);
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
    std::ostringstream rlog;
    rlog <<" **************************************************** " << std::endl;
    rlog <<" CHECK FOR fOLDER RENAME : " << fp.folder().foldername() << std::endl;
    Folder folder;
    if(file_manager_->GetFolderEntryByPostId(fp.id(), folder)) {
        rlog <<" FOLDER EXISTS : " << fp.folder().foldername() << std::endl;
        if(folder.foldername() != fp.folder().foldername()) {
            rlog <<" RENAMEING : " << folder.foldername() << std::endl;
            rlog <<" TO : " << fp.folder().foldername() << std::endl;
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
            rlog<<" old path : " << old_path << std::endl;
            rlog<<" new path : " << new_path << std::endl;

            std::string old_can, new_can;
            file_manager_->GetCanonicalPath(old_path, old_can);
            file_manager_->GetCanonicalPath(new_path, new_can);
            rlog <<" old canonical : " << old_can << std::endl;
            rlog <<" new canonical : " << new_can << std::endl;
            try {
                fs::RenamePath(old_can, new_can);
            }
            catch(std::exception& e) {
                rlog <<" EXCEPTION WHAT : " << e.what() << std::endl;
                log::LogException("rn_1809418204", e);

            }
        }

        if(folder.parent_post_id() != fp.folder().parent_post_id()) {
            rlog <<" PARENTS DIFFER " << std::endl;
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
            rlog <<" old path : " << old_path << std::endl;
            rlog <<" new path : " << new_path << std::endl;

            std::string old_can, new_can;
            file_manager_->GetCanonicalPath(old_path, old_can);
            file_manager_->GetCanonicalPath(new_path, new_can);
            rlog <<" old canonical : " << old_can << std::endl;
            rlog <<" new canonical : " << new_can << std::endl;
            try {
                fs::RenamePath(old_can, new_can);
            }
            catch(std::exception& e) {
                rlog <<" EXCEPTION WHAT : " << e.what() << std::endl;
                log::LogException("rn_18410915092", e);
            }
        }

    }

    rlog <<" **************************************************** " << std::endl;
    std::cout<< rlog.str() << std::endl;
    return ret;
}

// Checks FileInfo (preferably generated from a file post against the local cache)
bool RenameHandler::CheckForRename(FileInfo& fi, const std::string& post_id) {
    bool ret = false;
    std::ostringstream rlog;
    rlog <<" **************************************************** " << std::endl;
    rlog <<" CHECKING FOR RENAME " << std::endl;
    rlog <<" Incoming filepath : " << fi.filepath() << std::endl;
    FileInfo local_fi;
    if(file_manager_->GetFileInfoByPostId(post_id, local_fi)) {
        // If filepaths are the same
        if(local_fi.filepath() != fi.filepath()){
            std::cout<< " FILEPATHS DIFFER, RENAME " << std::endl;
            // Update Local cache
            std::string old_filepath = local_fi.filepath();
            std::string new_filepath = fi.filepath();

            rlog << "local filepath : " << local_fi.filepath() << std::endl;
            rlog << "post filepath : " << fi.filepath() << std::endl;
            int status = RenameFileLocalCache(old_filepath, new_filepath);
            rlog <<" new filepath : " << new_filepath << std::endl;
            if(status == ret::A_OK) {
                // Rename physical file
                std::string canonical_old, canonical_new;
                file_manager_->GetCanonicalPath(old_filepath, canonical_old);
                file_manager_->GetCanonicalPath(new_filepath, canonical_new);
                rlog <<" OLD : " << canonical_old << std::endl;
                rlog <<" NEW : " << canonical_new << std::endl;
                if(!canonical_new.empty()) {
                    if(fs::CheckFilepathExists(canonical_old)){
                        rlog <<" RENAMING " << std::endl;
                        fs::RenamePath(canonical_old, canonical_new);
                        ret = true;
                    }
                    else {
                        rlog <<" OLD PATH DOESNT EXIST " << std::endl;
                    }
                }
                else {
                    rlog <<" CANONICAL NEW IS EMPTY : " << canonical_new << std::endl;
                }
            }
            else {
                rlog <<" RENAME LOCAL CACHE FAILED " << std::endl;
            }
        }
    }

    rlog <<" CheckForRename status : " << ret << std::endl;
    rlog <<" **************************************************** " << std::endl;
    std::cout<< rlog.str() << std::endl;
    return ret;
}

}//namespace

