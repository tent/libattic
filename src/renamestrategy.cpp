#include "renamestrategy.h"

#include "errorcodes.h"
#include "filesystem.h"
#include "credentialsmanager.h"
#include "fileinfo.h"
#include "utils.h"
#include "jsonserializable.h"
#include "renamehandler.h"
#include "posthandler.h"
#include "filehandler.h"

namespace attic {

RenameStrategy::RenameStrategy() {}
RenameStrategy::~RenameStrategy() {}

int RenameStrategy::Execute(FileManager* pFileManager,
                            CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);
    // Initialize meta post
    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filetype = GetConfigValue("file_type");
    if(filetype == "file") {
        std::cout<<" RENAME FILE " << std::endl;
        status = RenameFile();
    }
    else if(filetype == "folder") {
        std::cout<<" RENAME FOLDER  " << std::endl;
        status = RenameFolder();
    }
    return status;
}

int RenameStrategy::RenameFile() {
    int status = ret::A_OK;
    std::string old_filepath = GetConfigValue("original_filepath");
    std::string new_filepath = GetConfigValue("new_filepath");
    std::string entity = GetConfigValue("entity");

    std::cout<<" Rename File : " << std::endl;
    std::cout<<"\t old filepath : " << old_filepath << std::endl;
    std::cout<<"\t new filepath : " << new_filepath << std::endl;

    RenameHandler rh(file_manager_);
    status = rh.RenameFileLocalCache(old_filepath, new_filepath);
    if(status == ret::A_OK) {
        FileInfo fi;
        if(file_manager_->GetFileInfo(new_filepath, fi)) {
            FilePost p;
            status = RetrieveFilePost(fi.post_id(), p);
            if(status == ret::A_OK) {
                std::string master_key;
                GetMasterKey(master_key);
                FileHandler fh(file_manager_);
                std::string cargo;
                fh.PrepareCargo(fi, master_key, cargo);
                FilePost new_p;
                rh.UpdateFileMetaPost(p, fi, new_p);
                new_p.set_cargo(cargo);
                status = UpdateFileMetaPost(fi.post_id(), new_p);
            }
        }
        else {
            std::cout<<" COULD NOT FIND FILE FOR PATH : " << new_filepath << std::endl;
            status = ret::A_FAIL_INVALID_FILE_INFO;
        }
    }

    std::cout<<" Rename File status : " << status << std::endl;

    return status;
}

int RenameStrategy::RenameFolder() {
    int status = ret::A_OK;
    std::string old_folderpath = GetConfigValue("original_folderpath"); 
    std::string new_folderpath = GetConfigValue("new_folderpath");
    std::string entity = GetConfigValue("entity");

    std::cout<<" original folderpath : " << old_folderpath << std::endl;
    std::cout<<" new folderpath : " << new_folderpath << std::endl;

    std::cout<<" RENAME FOLDER NOT IMPLEMENTED - RE IMPLEMENT " << std::endl;

    // Retreive folder entries for old folder

    /*
    RenameHandler rh(file_manager_);
    status = rh.RenameFolderLocalCache(old_folderpath, new_folderpath);
    if(status == ret::A_OK) {
        Folder folder;
        if(RetrieveFolder(new_folderpath, folder)) {
            FolderPost fp;
            // Retrieve Folder Post
            status = RetrieveFolderPost(folder.folder_post_id(), fp);
            if(status == ret::A_OK) {
                // Update it
                FolderPost new_fp;
                rh.UpdateFolderMetaPost(fp, folder, new_fp);
                status = UpdateFolderMetaPost(new_fp.id(), new_fp);
            }
        }
        else { 
            std::cout<<" COULD NOT FIND FOLDERPATH : " << new_folderpath << std::endl;
            status = ret::A_FAIL_INVALID_FOLDERPATH;
        }
    }
    */

    return status;
}

int RenameStrategy::RetrieveFolderPost(const std::string& post_id, FolderPost& fp) {
    int status = ret::A_OK;
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

    PostHandler<FolderPost> ph(access_token_);
    status = ph.Get(posturl, NULL, fp);
    return status;
}

int RenameStrategy::UpdateFolderMetaPost(const std::string& post_id, const FolderPost& fp) {
    int status = ret::A_OK;

    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
    FolderPost p = fp;

    PostHandler<FolderPost> ph(access_token_);
    status = ph.Put(posturl, NULL, p);

    std::cout<<" Update Folder Meta Post " << std::endl;
    std::cout<<" code : " << ph.response().code << std::endl;
    std::cout<<" body : " << ph.response().body << std::endl;
    return status;
}

int RenameStrategy::UpdateFileMetaPost(const std::string& post_id, const FilePost& fp) {
    int status = ret::A_OK;

    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
    FilePost p = fp;

    PostHandler<FilePost> ph(access_token_);
    status = ph.Put(posturl, NULL, p);

    std::cout<<" Update File Meta Post " << std::endl;
    std::cout<<" code : " << ph.response().code << std::endl;
    std::cout<<" body : " << ph.response().body << std::endl;
    return status;
}

int RenameStrategy::RetrieveFilePost(const std::string& post_id, FilePost& fp) {
    int status = ret::A_OK;
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

    PostHandler<FilePost> ph(access_token_);
    status = ph.Get(posturl, NULL, fp);
    return status;
}

bool RenameStrategy::RetrieveFolder(const std::string& folderpath, Folder& out) {
    return file_manager_->GetFolderEntry(folderpath, out);
}


} //namespace

