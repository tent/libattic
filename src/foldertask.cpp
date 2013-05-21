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
        RenameFolder();
    }

    Callback(status, operation);
    SetFinishedState();
}

int FolderTask::RenameFolder() {
    int status = ret::A_OK;
    std::string old_folderpath, new_folderpath;
    // These are absolute paths
    context_.get_value("original_folderpath", old_folderpath);
    context_.get_value("new_folderpath", new_folderpath);

    std::cout<<" renaming folder " << std::endl;
    std::cout<<" old : "<< old_folderpath << std::endl;
    std::cout<<" new : " << new_folderpath << std::endl;

    std::deque<FileInfo> file_list;
    std::deque<Folder> folder_list;

    FolderHandler fh(file_manager());
    fh.RenameFolder(old_folderpath, new_folderpath, file_list, folder_list);

    std::cout<<" updating posts ... " << std::endl;
    std::cout<<" folder size : " << folder_list.size() << std::endl;
    std::deque<Folder>::iterator folder_itr = folder_list.begin();
    for(;folder_itr != folder_list.end(); folder_itr++) {
        // Update Folder Post
        if(!(*folder_itr).folder_post_id().empty()) {
            FolderPost fp;
            status = RetrieveFolderPost((*folder_itr).folder_post_id(), fp);
            if(status == ret::A_OK) {
                std::cout<<" posting path : " << (*folder_itr).folderpath() << std::endl;
                fp.set_folder(*folder_itr);
                PostFolderPost((*folder_itr).folder_post_id(), fp);
            }
            else {
                std::cout<<" fialed to retrieve folder post " << std::endl;
            }
        }
        else {
            std::cout<<" EMPTY POST ID " << std::endl;
        }
    }

    std::cout<<" file size : " << file_list.size() << std::endl;
    std::deque<FileInfo>::iterator file_itr = file_list.begin();
    for(;file_itr != file_list.end(); file_itr++) {
        FilePost fp;
        fp.InitializeFilePost(&(*file_itr));
        if(!fp.id().empty()) {
            PostFilePost(fp.id(), fp);
        }
        else {
            std::cout<<" EMPTY POST ID " << std::endl;
        }
    }

    return status;
}

// TODO :: abstract this logic into one place, probably the folder handler
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
    std::deque<Folder>::iterator folder_itr = folder_list.begin();
    for(;folder_itr!= folder_list.end(); folder_itr++) {
        MarkFolderPostDeleted(*folder_itr);
    }
    // Set all file posts to deleted
    std::deque<FileInfo>::iterator file_itr = file_list.begin();
    for(;file_itr != file_list.end(); file_itr++) {
        MarkFilePostDeleted(*file_itr);
    }

    return status;
}

int FolderTask::MarkFolderPostDeleted(Folder& folder) {
    int status = ret::A_OK;
    std::string post_id = folder.folder_post_id();
    FolderPost fp;
    status = RetrieveFolderPost(post_id, fp);
    if(status == ret::A_OK) {
        fp.set_fragment(cnst::g_deleted_fragment);
        status = PostFolderPost(post_id, fp);
    }
    return status;
}

int FolderTask::MarkFilePostDeleted(FileInfo& fi) {
    int status = ret::A_OK;
    std::string post_id = fi.post_id();
    FilePost fp;
    status = RetrieveFilePost(post_id, fp);
    if(status == ret::A_OK) {
        fp.set_fragment(cnst::g_deleted_fragment);
        status = PostFilePost(post_id, fp);
    }
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
        std::string hold_id = cnst::g_szWorkingPlaceHolder; 
        std::deque<Folder>::iterator itr = folder_list.end();
        while(itr != folder_list.begin()) {
            std::cout<<" hold id : " << hold_id << std::endl;
            itr--;
            // Create Folder Post for each folder that needs it
            Folder folder = *itr;
            std::cout<<"folder path : " << folder.folderpath() << std::endl;
            if(folder.folder_post_id().empty()) {
                std::string post_id;
                folder.set_parent_post_id(hold_id);
                int s = CreateFolderPost(folder, post_id);
                if(s == ret::A_OK) { 
                    fh.SetFolderPostId(folder, post_id);
                    fh.SetFolderParentPostId(folder, hold_id);
                    hold_id = post_id;
                }
            }
            else {
                hold_id = folder.folder_post_id();
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

// TODO:: abstract to own class, retreive generic post perhaps templated
int FolderTask::RetrieveFilePost(const std::string& post_id, FilePost& out) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string post_path = GetPostPath();
        std::string posturl;
        utils::FindAndReplace(post_path, "{post}", post_id, posturl);
        std::cout<<" POST URL : " << posturl << std::endl;

        Response resp;
        netlib::HttpGet(posturl,
                        NULL,
                        &access_token(),
                        resp);

        if(resp.code == 200) {
            jsn::DeserializeObject(&out, resp.body);
        }
        else{
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("175kjas", resp);
        }
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

int FolderTask::PostFilePost(const std::string& post_id, FilePost& fp) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string post_path = GetPostPath();
        std::string posturl;
        utils::FindAndReplace(post_path, "{post}", post_id, posturl);
        std::cout<<" post url : " << posturl << std::endl;
        std::cout<<" post type : " << fp.type() << std::endl;

        Parent parent;
        parent.version = fp.version()->id();
        fp.PushBackParent(parent);

        std::string body;
        jsn::SerializeObject(&fp, body);
        Response resp;
        netlib::HttpPut(posturl,
                         fp.type(),
                         NULL,
                         body,
                         &access_token(),
                         resp);
        if(resp.code == 200) {
            std::cout<<" BODY : " << resp.body << std::endl;

        }
        else {
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("192151mm", resp);
        }
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

int FolderTask::RetrieveFolderPost(const std::string& post_id, FolderPost& out) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string post_path = GetPostPath();
        std::string posturl;
        utils::FindAndReplace(post_path, "{post}", post_id, posturl);
        std::cout<<" POST URL : " << posturl << std::endl;

        Response resp;
        netlib::HttpGet(posturl,
                        NULL,
                        &access_token(),
                        resp);

        if(resp.code == 200) {
            jsn::DeserializeObject(&out, resp.body);
        }
        else{
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("175kjas", resp);
        }
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

int FolderTask::PostFolderPost(const std::string& post_id, FolderPost& fp) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string post_path = GetPostPath();
        std::string posturl;
        utils::FindAndReplace(post_path, "{post}", post_id, posturl);
        std::cout<<" post url : " << posturl << std::endl;
        std::cout<<" post type : " << fp.type() << std::endl;
        std::cout<<" folderpath : " << fp.folder().folderpath() << std::endl;

        Parent parent;
        parent.version = fp.version()->id();
        fp.PushBackParent(parent);

        std::string body;
        jsn::SerializeObject(&fp, body);
        Response resp;
        netlib::HttpPut(posturl,
                         fp.type(),
                         NULL,
                         body,
                         &access_token(),
                         resp);
        if(resp.code == 200) {
            std::cout<<" BODY : " << resp.body << std::endl;

        }
        else {
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("192151mm", resp);
        }
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}


} //namespace

