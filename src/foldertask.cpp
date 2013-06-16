#include "foldertask.h"

#include "errorcodes.h"
#include "filehandler.h"
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
        std::string folderpath;
        context_.get_value("folderpath", folderpath);
//        CreateFolderHierarchy(folderpath);
        CreateFolder(folderpath);
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


    // Extract to-be parent
    // Check if exists
    //  if not, create tree
    //  Rename
    FolderHandler fh(file_manager());
    size_t p_pos = new_folderpath.rfind("/");
    if(p_pos != std::string::npos) {
        std::string parent = new_folderpath.substr(0, p_pos);
        std::cout<<" parent : " << parent << std::endl;
        if(!fh.IsFolderInCache(parent)) {
            CreateFolder(parent);
        }
    }

    std::deque<FileInfo> file_list;
    std::deque<Folder> folder_list;
    std::cout<<" RENAMING " << std::endl;
    fh.RenameFolder(old_folderpath, new_folderpath, file_list, folder_list);

    // Update folder post
    std::deque<Folder>::iterator folder_itr = folder_list.begin();
    for(;folder_itr != folder_list.end(); folder_itr++) {
        UpdateFolderPost((*folder_itr), (*folder_itr).folder_post_id());
    }

    // Update corresponding file posts
    std::cout<<" file size : " << file_list.size() << std::endl;
    std::deque<FileInfo>::iterator file_itr = file_list.begin();
    for(;file_itr != file_list.end(); file_itr++) {
        UpdateFilePost((*file_itr), (*file_itr).post_id());
    }

    return status;
}

bool FolderTask::UpdateFolderPost(Folder& folder, const std::string post_id) {
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
        err << " entry : " << folder.folderpath();
        err << " \t\t " << folder.folder_post_id();
        err << " \t\t " << folder.parent_post_id();
        log::LogString("folder_18912512", err.str());
    }
    return ret;
}

bool FolderTask::UpdateFilePost(FileInfo& fi, const std::string post_id) {
    bool ret = false;
    //  Retrieve latest
    if(!post_id.empty()) {
        FilePost p;
        if(RetrieveFilePost(post_id, p) == ret::A_OK) {
            std::string master_key;
            GetMasterKey(master_key);
            //  update cargo
            FileHandler fh(file_manager());
            std::string cargo;
            fh.PrepareCargo(fi, master_key, cargo);

            // set new filepath locally and in cargo


            p.set_cargo(cargo);
            //  post 
            if(PostFilePost(p.id(), p) == ret::A_OK)
                ret = true;
        }
    }
    else { 
        std::ostringstream err;
        err << " Empty File Post id : " << std::endl;
        err << " entry : " << fi.filepath();
        err << " \t\t " << fi.post_id();
        log::LogString("folder_18912512", err.str());
    }
    return ret;
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

int FolderTask::CreateFolder(const std::string& path) {
    int status = ret::A_OK;
    // Normalize path
    std::string folderpath = path;
    utils::CheckUrlAndRemoveTrailingSlash(folderpath);
    FolderHandler fh(file_manager());
    // Check if folder is deleted
    if(CheckFolderDeleted(folderpath)) {
        // Un-delete folder
        Folder folder;
        fh.GetFolder(folderpath, folder);
        folder.set_deleted(false);
        fh.SetFolderDeleted(folderpath, false);
        UpdateFolderPost(folder, folder.folder_post_id());
    }
    else {
        // Create Folder
        // Retrieve folder hierarchy
        std::deque<Folder> folder_list;
        fh.RetrieveFolders(folderpath, folder_list);
        std::cout<<" folder list size : " << folder_list.size() << std::endl;
        if(folder_list.size()) {
            // make sure posts exist for each parent folder
            std::string hold_id = cnst::g_szWorkingPlaceHolder; 
            std::deque<Folder>::iterator itr = folder_list.end();
            while(itr != folder_list.begin()) {
                itr--; // Its pointing to the end, decrement
                std::cout<<" hold id : " << hold_id << std::endl;
                std::cout<<" filepath : " << (*itr).folderpath() << std::endl;
                (*itr).set_parent_post_id(hold_id);
                if((*itr).folder_post_id().empty()) {
                    std::string post_id;
                    int s = CreateFolderPost((*itr), post_id);
                    if(s == ret::A_OK) { 
                        fh.InsertFolder(*itr);
                        hold_id = post_id;
                    }
                }
                else {
                    hold_id = (*itr).folder_post_id();
                }
            }
        }
    }

    return status;
}

bool FolderTask::CheckFolderDeleted(const std::string& folderpath) {
    bool ret = false;
    FolderHandler fh(file_manager());
    if(fh.IsFolderInCache(folderpath)) {
        Folder folder;
        fh.GetFolder(folderpath, folder);
        if(folder.deleted())
            ret = true;
    }
    return ret;
}

int FolderTask::CreateFolderHierarchyList(const std::string& folderpath, 
                                          std::deque<std::string>& out) {
    std::string working_path = file_manager()->working_directory();
    utils::CheckUrlAndAppendTrailingSlash(working_path);

    size_t pos  = folderpath.find(working_path);
    pos += working_path.size();
    if(pos != std::string::npos) {
        std::string path = folderpath.substr(pos);
        size_t ppos = 0;
        while(ppos != std::string::npos) {
            out.push_back(working_path + path);
            ppos = path.rfind("/");
            path = path.substr(0, ppos);
        }
        out.push_back(working_path);
    }
    /*
    std::deque<std::string>::iterator itr = folder_tree.begin();
    for(;itr!= folder_tree.end(); itr++)
        std::cout<< *itr << std::endl;
        */
    return ret::A_OK;
}

int FolderTask::CreateFolderPost(Folder& folder, std::string& id_out) {
    int status = ret::A_OK;
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed(); 
    // Create folderpost                                   
    FolderPost fp(folder);                                 
    PostHandler<FolderPost> ph(access_token());
    status = ph.Post(posts_feed, NULL, fp);
    if(status != ret::A_OK) {
        log::LogHttpResponse("lasp151", ph.response());
    }
    else  {
        std::cout << ph.response().body << std::endl;
        FolderPost p = ph.GetReturnPost();
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
        status = ph.Get(posturl, NULL, out);
        if(status != ret::A_OK)
            log::LogHttpResponse("175kjas", ph.response());
        else 
            std::cout << ph.response().body << std::endl;
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
        status = ph.Put(posturl, NULL, fp);
        if(status != ret::A_OK)
            log::LogHttpResponse("mas1o8", ph.response());
        else 
            std::cout << ph.response().body << std::endl;
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
        status = ph.Get(posturl, NULL, out);
        if(status != ret::A_OK)
            log::LogHttpResponse("asgwolf31", ph.response());
        else 
            std::cout << ph.response().body << std::endl;
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
        status = ph.Put(posturl, NULL, fp);
        if(status != ret::A_OK)
            log::LogHttpResponse("naven32109", ph.response());
        else 
            std::cout << ph.response().body << std::endl;
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}


} //namespace

