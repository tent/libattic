#include "postfolderstrategy.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "filesystem.h"
#include "folder.h"
#include "folderpost.h"
#include "folderhandler.h"
#include "logutils.h"
#include "posthandler.h"

namespace attic { 

int PostFolderStrategy::Execute(FileManager* pFileManager, CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);
    // Initialize meta post
    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filepath = GetConfigValue("filepath");
    std::string entity = GetConfigValue("entity");

    // absolute filepath
    if(!filepath.empty()) {
        //Extract Parent Directory
        std::string folderpath;
        status = fs::GetParentPath(filepath, folderpath);
        std::cout<<" FOLDER PATH : " << folderpath << std::endl;
        if(!folderpath.empty()) {
            std::string directory, directory_post_id;
            if(file_manager_->FindAssociatedWorkingDirectory(folderpath, 
                                                             directory, 
                                                             directory_post_id)) {
                FolderHandler fh(file_manager_);
                std::deque<std::string> names;
                if(fh.RetrieveFolders(folderpath, directory, names)) {
                    // validate each folder exists
                    std::deque<std::string>::iterator itr = names.begin();
                    std::string parent_post_id = directory_post_id;
                    for(;itr!=names.end();itr++) {
                        Folder folder;
                        if(!file_manager_->GetFolderEntry((*itr), parent_post_id, folder)) {
                            folder.set_foldername(*itr);
                            folder.set_parent_post_id(parent_post_id);
                            //  if not create post
                            std::string post_id;
                            CreateFolderPost(folder, post_id);
                            // Insert to table;
                            file_manager_->CreateFolderEntry(folder.foldername(),
                                                              folder.folder_post_id(),
                                                              folder.parent_post_id(),
                                                              folder);
                        }
                        else {
                            // Check if folderpath is deleted
                            if(file_manager_->IsFolderDeleted(folder.folder_post_id())){
                                // Un-delete
                                file_manager_->SetFolderDeleted(folder.folder_post_id(), false);
                                UpdateFolderPost(folder, folder.folder_post_id());
                            }
                        }
                        parent_post_id = folder.folder_post_id();
                    }
                    
                }
            }
        }
    }
    else {
        status = ret::A_FAIL_INVALID_FILEPATH;
    }
    std::cout<<" POST FODLER STRATEGY STATUS = " << status << std::endl;

    return status;
}

int PostFolderStrategy::CreateFolderPost(Folder& folder, std::string& id_out) {
    int status = ret::A_OK;
    // Create folderpost
    FolderPost fp(folder);
    PostHandler<FolderPost> ph(access_token_);
    status = ph.Post(posts_feed_, NULL, fp);
    if(status == ret::A_OK) {
        FolderPost back = ph.GetReturnPost();
        folder.set_folder_post_id(back.id());
        id_out = back.id();
    }
    else {
        log::LogHttpResponse("eskuus90___", ph.response());
    }
    return status;
}

bool PostFolderStrategy::UpdateFolderPost(Folder& folder, const std::string post_id) {
    bool ret = false;
    if(!post_id.empty()) {
        FolderPost fp;
        if(RetrieveFolderPost(folder.folder_post_id(), fp) == ret::A_OK) {
            fp.set_folder(folder);
            if(PostFolderPost(post_id, fp) == ret::A_OK)
                ret = true;
        }
    }
    else {
        std::ostringstream err;
        err << " Empty Folder Post id : " << std::endl;
        err << " entry : " << folder.foldername();
        err << " \t\t " << folder.folder_post_id();
        err << " \t\t " << folder.parent_post_id();
        log::LogString("pfs_18912512", err.str());
    }
    return ret;
}

int PostFolderStrategy::RetrieveFolderPost(const std::string& post_id, FolderPost& out) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string post_path = post_path_;
        std::string posturl;
        utils::FindAndReplace(post_path, "{post}", post_id, posturl);
        PostHandler<FolderPost> ph(access_token_);
        status = ph.Get(posturl, NULL, out);
        if(status != ret::A_OK)
            log::LogHttpResponse("pfs_13581", ph.response());
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

int PostFolderStrategy::PostFolderPost(const std::string& post_id, FolderPost& fp) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string post_path = post_path_;
        std::string posturl;
        utils::FindAndReplace(post_path, "{post}", post_id, posturl);
        PostHandler<FolderPost> ph(access_token_);
        status = ph.Put(posturl, NULL, fp);
        if(status != ret::A_OK)
            log::LogHttpResponse("pfs_n342309", ph.response());
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}


}//namespace
