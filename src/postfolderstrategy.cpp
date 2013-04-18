#include "postfolderstrategy.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "filesystem.h"
#include "folder.h"
#include "folderpost.h"
#include "netlib.h"

namespace attic { 

int PostFolderStrategy::Execute(FileManager* pFileManager, CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);
    // Initialize meta post
    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filepath = GetConfigValue("filepath");
    std::string entity = GetConfigValue("entity");

    std::cout<<" POST FOLDER STRATEGY " << std::endl;
    // Extract all parent directories up to the working directory
    std::vector<std::string> dirs;
    ExtractDirectories(filepath, dirs);

    std::vector<std::string>::iterator itr = dirs.begin();
    for(;itr != dirs.end(); itr++) {
        Folder folder;
        if(!file_manager_->GetFolderEntry(*itr, folder)) {
            // Create 
            file_manager_->CreateFolderEntry(*itr, "");
        }

        if(folder.folder_post_id().empty()) {
            // Create Folder Post
            status = CreateFolderPost(folder);
            if(status == ret::A_OK) {
                // Update folder post id;
                file_manager_->SetFolderPostId(*itr, folder.folder_post_id());
            }
        }
    }

    return status;
}

int PostFolderStrategy::CreateFolderPost(Folder& folder) {
    int status = ret::A_OK;

    // Create folderpost
    FolderPost fp(folder);
    std::string body;
    jsn::SerializeObject(&fp, body);
    // on success update post id in folder object
    Response response;
    netlib::HttpPost(posts_feed_,
                     fp.type(),
                     NULL,
                     body,
                     &access_token_,
                     response);

    if(response.code == 200) {
        FolderPost back;
        jsn::DeserializeObject(&back, response.body);
        folder.set_folder_post_id(back.id());
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

void PostFolderStrategy::ExtractDirectories(const std::string& filepath, 
                                            std::vector<std::string>& directories) {
    std::string working_dir = file_manager_->working_directory();
    std::string canonical;
    fs::GetCanonicalPath(filepath, canonical);

    std::cout<<" working dir : " << working_dir << std::endl;
    std::cout<<" filepath : "<< filepath << std::endl;
    std::cout<<" canonical : " << canonical << std::endl;
    
    directories.push_back(working_dir);
    utils::ExtractSubPaths(working_dir, canonical, directories);
}

}//namespace
