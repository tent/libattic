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

    std::cout<<" POST FOLDER STRATEGY " << std::endl;
    std::cout<<" filepath : " << filepath << std::endl;
    // absolute filepath
    if(!filepath.empty()) {
        //Extract Parent Directory
        std::string folderpath;
        status = fs::GetParentPath(filepath, folderpath);
        std::cout<<" folder path : " << folderpath << std::endl;
        if(!folderpath.empty()) {
            std::deque<Folder> folder_list;
            FolderHandler fh(file_manager_);
            
            if(fh.RetrieveFolders(folderpath, folder_list)){
                std::cout<<" total number of folders : " << folder_list.size() << std::endl;
                // Folder list comes out child -> parent in that order

                // top level folder's parent is working directory
                std::deque<Folder>::iterator itr = folder_list.end();

                std::string hold_id = cnst::g_szWorkingPlaceHolder; 
                while(itr!= folder_list.begin()) {
                    --itr;
                    std::cout<<" HOLD ID : " << hold_id << std::endl;
                    std::cout<<" processing folder : " << (*itr).folderpath() << std::endl;

                    // Set Parent post id;
                    if((*itr).parent_post_id().empty())    
                        (*itr).set_parent_post_id(hold_id);
                    else
                        std::cout<< " PARENT POST NOT EMPTY : " << (*itr).parent_post_id() << std::endl;
                    // Create new folder post
                    if((*itr).folder_post_id().empty()) {
                        std::cout<<"creating post for : " << (*itr).folderpath() << std::endl;
                        status = CreateFolderPost(*itr, hold_id); 
                        if(status == ret::A_OK) {
                            fh.InsertFolder(*itr);
                            hold_id = (*itr).folder_post_id();
                        }
                    }
                    else { 
                        // Post already exists
                        // set id for parent
                        hold_id = (*itr).folder_post_id();
                    }
                }
            }
            /*
            else {
                status = ret::A_FAIL_CREATE_DIRECTORY;
            }
            */
        }
    }
    else {
        status = ret::A_FAIL_INVALID_FILEPATH;
    }

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

}//namespace
