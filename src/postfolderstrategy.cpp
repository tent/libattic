#include "postfolderstrategy.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "filesystem.h"
#include "folder.h"
#include "folderpost.h"
#include "netlib.h"
#include "folderhandler.h"
#include "logutils.h"

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
            
            if(fh.CreateFolder(folderpath, folder_list)){
                std::cout<<" total number of folders : " << folder_list.size() << std::endl;
                // Folder list comes out child -> parent in that order

                // top level folder's parent is working directory
                std::deque<Folder>::iterator itr = folder_list.end();

                std::string hold_id = cnst::g_szWorkingPlaceHolder; 
                while(itr!= folder_list.begin()) {
                    --itr;
                    // Set Parent post id;
                    fh.SetFolderParentPostId(*itr, hold_id); //currently last id
                    // Create new folder post
                    if((*itr).folder_post_id().empty()) {
                        std::cout<<"creating post for : " << (*itr).folderpath() << std::endl;
                        CreateFolderPost(*itr, hold_id); 
                        // Set Folder Post Id;
                        if(!fh.SetFolderPostId(*itr, hold_id)) { 
                            std::cout<<" failed to set folder post id : " << hold_id << std::endl;
                        }
                    }
                    else { 
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

    std::cout<<" code : " << response.code << std::endl;
    std::cout<<" body : " << response.body << std::endl;

    if(response.code == 200) {
        FolderPost back;
        jsn::DeserializeObject(&back, response.body);
        folder.set_folder_post_id(back.id());
        id_out = back.id();
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

}//namespace
