#include "renamestrategy.h"

#include "errorcodes.h"
#include "filesystem.h"
#include "credentialsmanager.h"
#include "fileinfo.h"
#include "utils.h"
#include "netlib.h"
#include "jsonserializable.h"
#include "renamehandler.h"

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
        std::cout<<" RENAME FILE " << std::endl;
        status = RenameFile();
    }
    else if(filetype == "folder") {
        std::cout<<" RENAME FOLDER  " << std::endl;
        status = RenameFolder();
    }
    return status;
}

int RenameStrategy::RenameFile() {
    int status = ret::A_OK;
    std::string old_filepath = GetConfigValue("original_filepath");
    std::string new_filepath = GetConfigValue("new_filepath");
    std::string entity = GetConfigValue("entity");

    RenameHandler rh(file_manager_);
    status = rh.RenameFileLocalCache(old_filepath, new_filepath);
    if(status == ret::A_OK) {
        FileInfo* fi = RetrieveFileInfo(new_filepath);
        if(fi) {
            FilePost p;
            status = RetrieveFilePost(fi->post_id(), p);
            if(status == ret::A_OK) {
                FilePost new_p;
                rh.UpdateFileMetaPost(p, *fi, new_p);
                status = UpdateFileMetaPost(fi->post_id(), new_p);
            }
        }
        else {
            std::cout<<" COULD NOT FIND FILE FOR PATH : " << new_filepath << std::endl;
            status = ret::A_FAIL_INVALID_FILE_INFO;
        }
    }

    return status;
}

int RenameStrategy::RenameFolder() {
    int status = ret::A_OK;
    std::string old_folderpath = GetConfigValue("original_folderpath"); 
    std::string new_folderpath = GetConfigValue("new_folderpath");
    std::string entity = GetConfigValue("entity");

    std::cout<<" original folderpath : " << old_folderpath << std::endl;
    std::cout<<" new folderpath : " << new_folderpath << std::endl;

    RenameHandler rh(file_manager_);
    status = rh.RenameFolderLocalCache(old_folderpath, new_folderpath);
    if(status == ret::A_OK) {
        Folder folder;
        if(RetrieveFolder(new_folderpath, folder)) {
            FolderPost fp;
            // Retrieve Folder Post
            status = RetrieveFolderPost(folder.folder_post_id(), fp);
            if(status == ret::A_OK) {
                // Update it
                FolderPost new_fp;
                rh.UpdateFolderMetaPost(fp, folder, new_fp);
                status = UpdateFolderMetaPost(new_fp.id(), new_fp);
            }

        }
        else { 
            std::cout<<" COULD NOT FIND FOLDERPATH : " << new_folderpath << std::endl;
            status = ret::A_FAIL_INVALID_FOLDERPATH;
        }
    }

    return status;
}

int RenameStrategy::RetrieveFolderPost(const std::string& post_id, FolderPost& fp) {
    int status = ret::A_OK;

    std::cout<<" RETRIEVE FOLDER POST " << std::endl;

    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
    std::cout<<" POST URL : " << posturl << std::endl;

    Response response;
    netlib::HttpGet(posturl,
                    NULL,
                    &access_token_,
                    response);
    std::cout<<" CODE : " << response.code << std::endl;
    std::cout<<" BODY : " << response.body << std::endl;
    if(response.code == 200) {
        jsn::DeserializeObject(&fp, response.body);
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int RenameStrategy::UpdateFolderMetaPost(const std::string& post_id, const FolderPost& fp) {
    int status = ret::A_OK;
    std::string body;
    FolderPost p = fp;
    jsn::SerializeObject(&p, body);

    status = UpdatePost(post_id, fp.type(), body);
    return status;
}

int RenameStrategy::UpdateFileMetaPost(const std::string& post_id, const FilePost& fp) {
    int status = ret::A_OK;
    std::string body;
    FilePost p = fp;
    jsn::SerializeObject(&p, body);

    status = UpdatePost(post_id, fp.type(), body);
    return status;
}

int RenameStrategy::UpdatePost(const std::string& post_id, 
                               const std::string& post_type, 
                               const std::string& body) {
    int status = ret::A_OK;

    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
    std::cout<<" POST URL : " << posturl << std::endl;
    Response resp;
    netlib::HttpPut(posturl,
                    post_type,
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

bool RenameStrategy::RetrieveFolder(const std::string& folderpath, Folder& out) {
    return file_manager_->GetFolderEntry(folderpath, out);
}


} //namespace

