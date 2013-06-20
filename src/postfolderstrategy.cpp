#include "postfolderstrategy.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "filesystem.h"
#include "folder.h"
#include "folderpost.h"
#include "folderhandler.h"
#include "logutils.h"
#include "posthandler.h"
#include "foldercreationlock.h"

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
        std::cout<< "$$ Post filepath : " << filepath << std::endl;
        std::string folderpath;
        status = fs::GetParentPath(filepath, folderpath);

        fcl flocker(); // Create instance to hold ref
        FolderHandler fh(file_manager_);
        if(!fh.ValidateFolderPath(folderpath,
                                  posts_feed_,
                                  post_path_,
                                  access_token_)) {
            status = ret::A_FAIL_VALIDATE_DIRECTORY;
        }
    }
    else {
        status = ret::A_FAIL_INVALID_FILEPATH;
    }

    return status;
}


}//namespace
