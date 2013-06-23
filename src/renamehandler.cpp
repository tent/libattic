#include "renamehandler.h"

#include <iostream>

#include "netlib.h"
#include "filemanager.h"
#include "filesystem.h"
#include "logutils.h"
#include "foldersem.h"

namespace attic { 

RenameHandler::RenameHandler(FileManager* fi) {
    file_manager_ = fi;
}

RenameHandler::~RenameHandler() {
    file_manager_ = NULL;
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
                FolderSem fs;
                fs.AquireWrite(fp.id());
                fs::RenamePath(old_can, new_can);
                fs.ReleaseWrite(fp.id());
                ret = true;
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
                FolderSem fs;
                fs.AquireWrite(fp.id());
                fs::RenamePath(old_can, new_can);
                fs.ReleaseWrite(fp.id());
                ret = true;
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

bool RenameHandler::RenameFileLocalCacheAbsolutePath(const std::string& absolute_path, 
                                                     const std::string& new_filename) {
    bool ret = false;
    // check if it exists
    if(fs::CheckFilepathExists(absolute_path)) {
        // get paret dir
        std::string folderpath;
        if(fs::GetParentPath(absolute_path, folderpath)) {
            Folder folder;
            if(file_manager_->GetFolderEntry(folderpath, folder)){
                ret = RenameFileLocalCache(folder.folder_post_id(), new_filename);
            }
        }
    }

    return ret;
}


bool RenameHandler::RenameFileLocalCache(const std::string& post_id, 
                                         const std::string& new_filename) {
    bool ret = false;
    FileInfo fi;
    if(file_manager_->GetFileInfoByPostId(post_id, fi)) {
        // Construct filepath for current local cache
        std::string folderpath, canonical;
        if(file_manager_->ConstructFolderpath(fi.folder_post_id(), folderpath)) {
            file_manager_->GetCanonicalPath(folderpath, canonical);
            // Construct filepath for updated local cache
            std::string old_filepath, new_filepath;
            utils::AppendTrailingSlash(canonical);
            old_filepath = canonical + fi.filename();
            new_filepath = canonical + new_filename;
            // perform rename
            if(fs::CheckFilepathExists(old_filepath)){
                if(!fs::CheckFilepathExists(new_filepath)) {
                    FolderSem fs;
                    fs.AquireWrite(fi.folder_post_id());
                    if(fs::RenamePath(old_filepath, new_filepath)) {
                        file_manager_->SetFilename(post_id, new_filename);
                        ret = true;
                    }
                    fs.ReleaseWrite(fi.folder_post_id());
                }
            }
        }
    }
    return ret;
}

}//namespace

