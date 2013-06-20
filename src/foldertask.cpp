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

    std::cout<<" renaming folder " << std::endl;
    std::cout<<" old : "<< old_folderpath << std::endl;
    std::cout<<" new : " << new_folderpath << std::endl;

    // make sure old folder exists
    //  // retrieve folder data
    // make sure both paths have common root
    std::string directory, dir_post_id;
    if(HaveCommonWorkingDir(old_folderpath, new_folderpath, directory, dir_post_id)){
        // separeate new path into list
        FolderHandler fh(file_manager());
        std::deque<std::string> names;
        if(fh.RetrieveFolders(new_folderpath, directory, names)) {
            //  - retrieve folder data for each, 
            //  - create new folder if folder doesn't yet exist
            //  - rename old folder (name)
            //  - reparent if necessary
            //  - update folderpost
            std::deque<std::string>::iterator itr = names.begin();
            std::string parent_post_id = dir_post_id;
            for(;itr!=names.end();itr++) {
                Folder folder;
                if(*itr == names.back()) {
                    // this is the folder we are renaming to
                    file_manager()->GetFolderEntry(old_folderpath, folder);
                    // Update folder
                    folder.set_parent_post_id(parent_post_id);
                    std::cout<<" old folder name : " << folder.foldername() << std::endl;
                    folder.set_foldername(*itr);
                    std::cout<<" new folder name : " << (*itr) << std::endl;
                    file_manager()->SetFolderParentPostId(folder.folder_post_id(),
                                                          parent_post_id); // new parent post id
                    file_manager()->SetFoldername(folder.folder_post_id(),
                                                  (*itr));
                    UpdateFolderPost(folder, folder.folder_post_id());
                }
                else {
                    std::cout<<" ATTEMPTING RENAME CREATE " << std::endl;
                    if(!file_manager()->GetFolderEntry((*itr), parent_post_id, folder)) {
                        folder.set_foldername(*itr);
                        folder.set_parent_post_id(parent_post_id);
                        //  if not create post
                        std::string post_id;
                        CreateFolderPost(folder, post_id);
                        // Insert to table;
                        file_manager()->CreateFolderEntry(folder.foldername(),
                                                          folder.folder_post_id(),
                                                          folder.parent_post_id(),
                                                          folder);
                    }
                    else {
                        // Check if folderpath is deleted
                        if(file_manager()->IsFolderDeleted(folder.folder_post_id())){
                            // Un-delete
                            file_manager()->SetFolderDeleted(folder.folder_post_id(), false);
                            UpdateFolderPost(folder, folder.folder_post_id());
                        }
                    }
                }
                parent_post_id = folder.folder_post_id();
            }
        }
    }
    else {
        status = ret::A_FAIL_DIFFERING_WORK_DIRECTORY;
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
        err << " entry : " << folder.foldername();
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

    std::cout<<" @@@ CREATING FOLDER : " << path << std::endl;

    FolderHandler fh(file_manager());
    if(fh.ValidateFolderPath(path,
                             entity().GetPreferredServer().posts_feed(),
                             entity().GetPreferredServer().post(),
                             access_token())) {
        status = ret::A_FAIL_VALIDATE_DIRECTORY;
    }
    return status;
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
        std::cout<<" foldername : " << fp.folder().foldername() << std::endl;

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

bool FolderTask::HaveCommonWorkingDir(const std::string& old_folderpath, 
                                      const std::string& new_folderpath, 
                                      std::string& directory_out,
                                      std::string& post_id_out) {
    bool ret = false;
    std::string directory, directory_post_id;
    if(file_manager()->FindAssociatedWorkingDirectory(old_folderpath, 
                                                      directory, 
                                                      directory_post_id)) {
        std::string new_dir, new_dir_id;
        if(file_manager()->FindAssociatedWorkingDirectory(new_folderpath, 
                                                          new_dir, 
                                                          new_dir_id)) {
            if(directory == new_dir) {
                ret = true;
                directory_out = directory;
                post_id_out = directory_post_id;
            }
        }
    }
    return ret;
}

} //namespace

