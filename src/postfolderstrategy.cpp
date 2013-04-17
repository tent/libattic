#include "postfolderstrategy.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "filesystem.h"

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
        // Check if there is a folder entry for each
        //      if yes, do nothing
        //      else, create one
    }

    return status;
}

void PostFolderStrategy::ExtractDirectories(const std::string& filepath, 
                                            std::vector<std::string>& directories) {
    std::string working_dir;
    file_manager_->GetWorkingDirectory(working_dir);

    std::string canonical;
    fs::GetCanonicalPath(filepath, canonical);

    std::cout<<" working dir : " << working_dir << std::endl;
    std::cout<<" filepath : "<< filepath << std::endl;
    std::cout<<" canonical : " << canonical << std::endl;
    
    utils::ExtractSubPaths(working_dir, canonical, directories);
}

}//namespace
