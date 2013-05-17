#include "foldertask.h"

#include "errorcodes.h"
#include "netlib.h"
#include "folderhandler.h"

namespace attic {
FolderTask::FolderTask(FileManager* pFm,
                       CredentialsManager* pCm,
                       const AccessToken& at,
                       const Entity& entity,
                       const TaskContext& context)
                       :
                       TentTask(Task::FOLDER,
                                pFm,
                                pCm,
                                at,
                                entity,
                                context) {

}


FolderTask::~FolderTask() {}

void FolderTask::RunTask() {
    int status = ret::A_OK;
    std::cout<<" FOLDER TASK " << std::endl;
    std::string operation;
    context_.get_value("operation", operation);
    std::cout<<"\tfolder operation : " << operation << std::endl;

    if(operation == "CREATE") {
        CreateFolder();
    }
    else if(operation == "DELETE") {
        DeleteFolder();
    }
    else if(operation == "RENAME") {

    }

    Callback(status, operation);
    SetFinishedState();
}

int FolderTask::DeleteFolder() {
    int status = ret::A_OK;
    std::string folderpath;
    context_.get_value("folderpath", folderpath);

    FolderHandler fh(file_manager());
    std::deque<FileInfo> file_list;
    std::deque<Folder> folder_list;
    fh.DeleteFolder(folderpath, file_list, folder_list);

    std::cout << " file list count : " << file_list.size() << std::endl;
    std::cout << " folder list count : " << folder_list.size() << std::endl;
    // Set all folder posts to deleted
    // Set all file posts to deleted

    return status;
}

int FolderTask::CreateFolder() {
    int status = ret::A_OK;
    std::string folderpath;
    context_.get_value("folderpath", folderpath);

    FolderHandler fh(file_manager());
    std::deque<Folder> folder_list;
    fh.CreateFolder(folderpath, folder_list);
    std::cout<<" folder list size : " << folder_list.size() << std::endl;
    if(folder_list.size()) {
        std::deque<Folder>::iterator itr = folder_list.begin();
        for(;itr!=folder_list.end(); itr++) {
            // Create Folder Post for each folder that needs it
            Folder folder = *itr;
            std::cout<<"folder path : " << folder.folderpath() << std::endl;
            if(folder.folder_post_id().empty()) {
                std::string post_id;
                int s = CreateFolderPost(folder, post_id);
                if(s == ret::A_OK) 
                    fh.SetFolderPostId(folder, post_id);
            }
        }
    }
    return status;
}

int FolderTask::CreateFolderPost(Folder& folder, std::string& id_out) {
    int status = ret::A_OK;
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed(); 
    // Create folderpost                                   
    FolderPost fp(folder);                                 
    std::string body;                                      
    jsn::SerializeObject(&fp, body);                       
    // on success update post id in folder object          
    Response response;                                     
    netlib::HttpPost(posts_feed,
                     fp.type(),
                     NULL,
                     body,
                     &access_token(),
                     response);

    std::cout<<" FOLDER RESPONSE : " << std::endl;
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


} //namespace

