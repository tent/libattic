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
#include "folderhandler.h"

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
    entity_ = GetConfigValue("entity");

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
    // Must be absolute paths
    std::string old_filepath = GetConfigValue("original_filepath");
    std::string new_filepath = GetConfigValue("new_filepath");

    std::ostringstream rlog;
    rlog << "******************************************************" << std::endl;
    rlog <<" Rename File : " << std::endl;
    rlog <<"\t old filepath : " << old_filepath << std::endl;
    rlog <<"\t new filepath : " << new_filepath << std::endl;

    // Determine common working directory
    // find corresponding parent folder and thus file info
    // update meta post
    //  - upon success make change locally
    std::string working_directory, working_post_id;
    if(HaveCommonWorkingDir(old_filepath, new_filepath, working_directory, working_post_id)) {
        // Validate that the old filepath exists
            // assuming the system has already taken care of the rename on this side,
            // we just need to update the metadata
        // remove filename
        size_t pos = new_filepath.rfind("/");
        if(pos != std::string::npos) {
            std::string new_folderpath = new_filepath.substr(0, pos);
            rlog << "\t new folderpath : " << new_folderpath << std::endl;
            std::string old_folderpath;
            fs::GetParentPath(old_filepath, old_folderpath);
            rlog << "\t old folderpath : " << old_folderpath << std::endl;
            // Compare working directories,
            if(new_folderpath != old_folderpath) {
                FolderHandler fh(file_manager_);
                //  - if moving to another directory
                //      - validate new path
                if(!fh.ValidateFolderPath(new_folderpath,
                                          entity_,
                                          posts_feed_,
                                          post_path_,
                                          access_token_)) {
                    status = ret::A_FAIL_VALIDATE_DIRECTORY;
                }
            }

            if(status == ret::A_OK) {
                status = UpdateFileMetaData(old_filepath,
                                            new_filepath);
            }
        }
        else {
            status = ret::A_FAIL_INVALID_FILEPATH;
        }
    }
    else {
        status = ret::A_FAIL_DIFFERING_WORK_DIRECTORY;
    }

    rlog << "Rename File status : " << status << std::endl;
    rlog << "******************************************************" << std::endl;
    std::cout<< rlog.str() << std::endl;

    return status;
}

int RenameStrategy::UpdateFileMetaData(const std::string& old_filepath,
                                       const std::string& new_filepath) {
    int status = ret::A_OK;

    std::ostringstream rlog;
    rlog << "******************************************************" << std::endl;
    rlog << " UpdateFileMetaData" << status << std::endl;
    // Extract parent folderpaths
    std::string old_folderpath;
    fs::GetParentPath(old_filepath, old_folderpath);

    std::string new_filename, new_folderpath;
    size_t pos = new_filepath.rfind("/");
    if(pos != std::string::npos) {
       new_filename = new_filepath.substr(pos+1); // get filepath
       new_folderpath = new_filepath.substr(0, pos); // get folderpath
    }

    rlog << " new filename : " << new_filename << std::endl;
    rlog << " new folderpath : " << new_folderpath << std::endl;
    rlog << " old folderpath : " << old_folderpath << std::endl;

    // Retreive folder and attempt to get file info
    Folder folder;
    FolderHandler fh(file_manager_);
    fh.GetFolderByAbsolutePath(old_folderpath, folder);
    size_t fn_pos = old_filepath.rfind("/");
    if(fn_pos != std::string::npos) {
        FileInfo fi;
        rlog << " old filename : " << old_filepath.substr(fn_pos+1) << std::endl;
        rlog << " folder post id : " << folder.folder_post_id() << std::endl;
        file_manager_->GetFileInfo(old_filepath.substr(fn_pos+1),
                                   folder.folder_post_id(),
                                   fi);
        // update post
        rlog << " file post id : " << fi.post_id() << std::endl;
        FilePost p;
        status = RetrieveFilePost(fi.post_id(), p);
        rlog << " retrieveing file post status : " << status << std::endl;
        if(status == ret::A_OK) {
            // update file info
            //  - filepath
            //      - possibly folder post
            //  - filename
            std::string aliased_path;
            file_manager_->GetAliasedPath(new_filepath, aliased_path);
            fi.set_filepath(aliased_path);
            fi.set_filename(new_filename);
            if(new_folderpath != old_folderpath) {
                rlog << " folder paths differ setting new folder post id " << std::endl;
                // update folderpost
                Folder new_folder;
                if(fh.GetFolderByAbsolutePath(new_folderpath, new_folder)) {
                    rlog << " new folder post id : " << new_folder.folder_post_id() << std::endl;
                    rlog << " new foldername : " << new_folder.foldername() << std::endl;
                    fi.set_folder_post_id(new_folder.folder_post_id());
                }
            }

            RenameHandler rh(file_manager_);
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

        if(status == ret::A_OK) {
            //if(fs::RenamePath(old_filepath, new_filepath)) { } // assuming the system has done this
            // Update meta data
            file_manager_->SetFilepath(p.id(), fi.filepath());
            file_manager_->SetFilename(p.id(), fi.filename());
            file_manager_->SetFolderPostId(p.id(), fi.folder_post_id());
            //else {
            //    status = ret::A_FAIL_RENAME_FILE;
            //}
        }
    }

    rlog << " UpdateFileMetaData status : " << status << std::endl;
    rlog << "******************************************************" << std::endl;
    std::cout << rlog.str() << std::endl;
    return status;
}


/*
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
*/

int RenameStrategy::RenameFolder() {
    int status = ret::A_OK;
    std::string old_folderpath = GetConfigValue("original_folderpath"); 
    std::string new_folderpath = GetConfigValue("new_folderpath");
    std::string entity = GetConfigValue("entity");

    std::cout<<" original folderpath : " << old_folderpath << std::endl;
    std::cout<<" new folderpath : " << new_folderpath << std::endl;

    std::cout<<" RENAME FOLDER NOT IMPLEMENTED - RE IMPLEMENT " << std::endl;


    // TODO :: move logic from foldertask here and have folder task use the rename strategy

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

    std::cout<<" Retrieve File Post url : " << posturl << std::endl;
    PostHandler<FilePost> ph(access_token_);
    status = ph.Get(posturl, NULL, fp);
    return status;
}

bool RenameStrategy::RetrieveFolder(const std::string& folderpath, Folder& out) {
    return file_manager_->GetFolderEntry(folderpath, out);
}

bool RenameStrategy::HaveCommonWorkingDir(const std::string& old_folderpath, 
                                          const std::string& new_folderpath, 
                                          std::string& directory_out,
                                          std::string& post_id_out) {
    bool ret = false;
    std::string directory, directory_post_id;
    if(file_manager_->FindAssociatedWorkingDirectory(old_folderpath, 
                                                      directory, 
                                                      directory_post_id)) {
        std::string new_dir, new_dir_id;
        if(file_manager_->FindAssociatedWorkingDirectory(new_folderpath, 
                                                          new_dir, 
                                                          new_dir_id)) {
            if(directory == new_dir) {
                ret = true;
                directory_out = directory;
                post_id_out = directory_post_id;
            }
        }
    }
    return ret;
}


} //namespace

