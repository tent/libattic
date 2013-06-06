#include "foldertask.h"

#include "errorcodes.h"
#include "folderhandler.h"
#include "posthandler.h"

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
    // Normalize folderpath
    utils::CheckUrlAndRemoveTrailingSlash(old_folderpath);
    utils::CheckUrlAndRemoveTrailingSlash(new_folderpath);

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
        std::cout<<" file relative path : " << fp.relative_path() << std::endl;
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
    // Normalize folderpath
    utils::CheckUrlAndRemoveTrailingSlash(folderpath);

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
    std::cout<<" creating folder ... " << std::endl;
    // Normalize folderpath
    utils::CheckUrlAndRemoveTrailingSlash(folderpath);
    FolderHandler fh(file_manager());
    // Check if folder posts was previously deleted
    if(fh.IsFolderInCache(folderpath)) {
        std::cout<<" folder is in cache " << std::endl;
        Folder folder;
        fh.GetFolder(folderpath, folder);
        if(folder.deleted()) {
            std::cout<<" folder deleted " << std::endl;
            // "Un-delete" folder (post)
            folder.set_deleted(false);
            fh.SetFolderDeleted(folderpath, false);
            // Update post
            FolderPost fp;
            status = RetrieveFolderPost(folder.folder_post_id(), fp);
            if(status == ret::A_OK) {
                fp.set_folder(folder);
                fp.clear_fragment();
                status = PostFolderPost(folder.folder_post_id(), fp);
            }
        }
        else {
            std::cout<<" duplicate post creation " << std::endl;
        }
    }
    else {
        std::deque<Folder> folder_list;
        fh.RetrieveFolders(folderpath, folder_list);
        std::cout<<" folder list size : " << folder_list.size() << std::endl;
        if(folder_list.size()) {
            std::string hold_id = cnst::g_szWorkingPlaceHolder; 
            std::deque<Folder>::iterator itr = folder_list.end();
            while(itr != folder_list.begin()) {
                std::cout<<" hold id : " << hold_id << std::endl;
                itr--;
                // Create Folder Post for each folder that needs it
                Folder folder = *itr;
                if(folder.parent_post_id().empty())
                    folder.set_parent_post_id(hold_id);
                std::cout<<"folder path : " << folder.folderpath() << std::endl;
                if(folder.folder_post_id().empty()) {
                    std::string post_id;
                    int s = CreateFolderPost(folder, post_id);
                    if(s == ret::A_OK) { 
                        fh.InsertFolder(folder);
                        hold_id = post_id;
                    }
                }
                else {
                    hold_id = folder.folder_post_id();
                }
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

    PostHandler<FolderPost> ph(access_token());
    Response response;                                     
    status = ph.Post(posts_feed, NULL, fp, response);

    if(status != ret::A_OK) {
        log::LogHttpResponse("lasp151", response);
    }
    else  {
        std::cout << response.body << std::endl;
        FolderPost p;
        jsn::DeserializeObject(&p, response.body);
        folder = p.folder();
        id_out = p.id();
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

        PostHandler<FilePost> ph(access_token());
        Response response;
        status = ph.Get(posturl, NULL, out, response);
        if(status != ret::A_OK)
            log::LogHttpResponse("175kjas", response);
        else 
            std::cout << response.body << std::endl;
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


        PostHandler<FilePost> ph(access_token());
        Response response;
        status = ph.Put(posturl, NULL, fp, response);
        if(status != ret::A_OK)
            log::LogHttpResponse("mas1o8", response);
        else 
            std::cout << response.body << std::endl;
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

        PostHandler<FolderPost> ph(access_token());
        Response response;
        status = ph.Get(posturl, NULL, out, response);
        if(status != ret::A_OK)
            log::LogHttpResponse("asgwolf31", response);
        else 
            std::cout << response.body << std::endl;
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

        PostHandler<FolderPost> ph(access_token());
        Response response;
        status = ph.Put(posturl, NULL, fp, response);
        if(status != ret::A_OK)
            log::LogHttpResponse("naven32109", response);
        else 
            std::cout << response.body << std::endl;
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}


} //namespace

