#include "renamestrategy.h"

#include "errorcodes.h"
#include "filesystem.h"
#include "credentialsmanager.h"
#include "fileinfo.h"
#include "utils.h"
#include "netlib.h"
#include "jsonserializable.h"

namespace attic {

RenameStrategy::RenameStrategy() {}
RenameStrategy::~RenameStrategy() {}

int RenameStrategy::Execute(FileManager* pFileManager,
                            CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);
    // Initialize meta post
    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filetype = GetConfigValue("file_type");
    if(filetype == "file") {
        status = RenameFile();
    }
    else if(filetype == "folder") {
        status = RenameFolder();
    }
    return status;
}

int RenameStrategy::RenameFile() {
    int status = ret::A_OK;
    std::string old_filepath = GetConfigValue("original_filepath");
    std::string new_filepath = GetConfigValue("new_filepath");
    std::string entity = GetConfigValue("entity");

    FileInfo* fi = RetrieveFileInfo(old_filepath);
    if(fi) {
        // Update File Info
        //
        std::string new_filename;
        utils::ExtractFileName(new_filepath, new_filename);
        std::cout<< " new filename : " << new_filename << std::endl;

        status = file_manager_->RenameFile(old_filepath, new_filename);
        std::cout<<" RENAME LOCAL CACHE FILE STATUS : " << status << std::endl;
        std::string relative;
        std::cout<<" NEW FILEPATH : " << new_filepath << std::endl;
        std::string parent_dir;
        fs::GetParentPath(new_filepath, parent_dir);
        std::cout<<" Parent dir : " << parent_dir << std::endl;
        std::cout<<" filename : " << new_filename << std::endl;

        file_manager_->GetRelativePath(parent_dir, relative);
        relative +=  new_filename;
        std::cout<<" GET RELATIVE : " << relative << std::endl;
        
        // Update meta post
        std::string meta_post_id = fi->post_id();
        status = UpdateFileMetaPost(meta_post_id,
                                    new_filepath,
                                    relative);
        
    }
    else {
        status = ret::A_FAIL_INVALID_FILE_INFO;
    }
    return status;
}

int RenameStrategy::RenameFolder() {
    int status = ret::A_OK;
    std::string old_folderpath = GetConfigValue("original_folderpath"); 
    std::string new_folderpath = GetConfigValue("new_folderpath");
    std::string entity = GetConfigValue("entity");

    std::string folder_post_id;
    Folder folder;
    if(file_manager_->GetFolderEntry(old_folderpath, folder)) {
        status = UpdateFolderMetaPost(folder_post_id, 
                                      new_folderpath, 
                                      folder);
        if(status == ret::A_OK){
            if(!file_manager_->UpdateFolderEntry(folder)) {
                status = ret::A_FAIL_UPDATE_MANIFEST;
            }
        }
    }

    return status;
}

int RenameStrategy::RetrieveFolderPost(const std::string& post_id, FolderPost& fp) {
    int status = ret::A_OK;

    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
    std::cout<<" POST URL : " << posturl << std::endl;

    Response response;
    netlib::HttpGet(posturl,
                    NULL,
                    &access_token_,
                    response);
    if(response.code == 200) {
        jsn::DeserializeObject(&fp, response.body);
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int RenameStrategy::UpdateFolderMetaPost(const std::string& post_id,
                                         const std::string& folderpath, 
                                         Folder& folder) { 
    int status = ret::A_OK;
    FolderPost fp;
    status = RetrieveFolderPost(folder.folder_post_id(), fp);
    if(status == ret::A_OK) {
        folder.set_folderpath(folderpath);
        fp.set_folder(folder);
    }

    return status;
}

int RenameStrategy::UpdateFileMetaPost(const std::string& post_id, 
                                       const std::string& filename,
                                       const std::string& relative_path) {
    int status = ret::A_OK;
    FilePost fp;
    status = RetrieveFilePost(post_id, fp);
    if(status == ret::A_OK) {
        fp.set_relative_path(relative_path);
        std::cout<<" FILENAME : " << filename << std::endl;
        fp.set_name(filename);

        Parent parent;
        parent.version = fp.version()->id;
        fp.PushBackParent(parent);
        
        std::string body;
        jsn::SerializeObject(&fp, body);

        std::string posturl;
        utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
        std::cout<<" POST URL : " << posturl << std::endl;
        Response resp;
        netlib::HttpPut(posturl,
                        fp.type(),
                        NULL,
                        body,
                        &access_token_,
                        resp);

        if(resp.code == 200) {

        }
        else {
            status = ret::A_FAIL_NON_200;
        }
        std::cout<<" code : " << resp.code << std::endl;
        std::cout<<" body : " << resp.body << std::endl;
    }
    return status;
}

int RenameStrategy::RetrieveFilePost(const std::string& post_id, FilePost& fp) {
    int status = ret::A_OK;
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
    std::cout<<" POST URL : " << posturl << std::endl;

    Response response;
    netlib::HttpGet(posturl,
                    NULL,
                    &access_token_,
                    response);
    if(response.code == 200) {
        jsn::DeserializeObject(&fp, response.body);
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

FileInfo* RenameStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = file_manager_->GetFileInfo(filepath);
    if(!fi)
        fi = file_manager_->CreateFileInfo();
    return fi;
}


} //namespace

