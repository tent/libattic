#include "postmetastrategy.h"

#include "filehandler.h"
#include "folderhandler.h"
#include "posthandler.h"

namespace attic {

int PostMetaStrategy::Execute(FileManager* fm, CredentialsManager* cm) {
    int status = ret::A_OK;
    status = InitInstance(fm, cm);

    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filepath = GetConfigValue("filepath");
    std::string entity = GetConfigValue("entity");

    std::cout<<" Post meta strategy " << std::endl;
    if(ValidMasterKey()) {
        FileHandler fh(file_manager_);
        if(!fh.DoesFileExist(filepath)) {
            std::cout<<" File doesn't exist yet, create meta post" << std::endl;
            FileInfo fi;
            status = CreateFileEntry(filepath, fi);
        }
        else {
           // no nead to throw an error, postfilestrategy should dif the hashes 
           // and really determine if it should acutally be re-uploaded
           //
           // status = ret::A_FAIL_FILE_ALREADY_EXISTS;
        }
    }
    else {
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    std::cout<<" post meta strategy status : " << status << std::endl;
    return status;
}

int PostMetaStrategy::CreateFileEntry(const std::string& filepath, FileInfo& out) {
    int status = ret::A_OK;
    std::cout<<" creating file entry : " << filepath << std::endl;
    std::string mk;
    GetMasterKey(mk);
    FileHandler fh(file_manager_);
    if(!fh.CreateNewFile(filepath, mk, posts_feed_, access_token_, out))
        status = ret::A_FAIL_CREATE_FILE_INFO;
    return status;
}

bool PostMetaStrategy::RetrieveFolderPostId(const std::string& filepath, std::string& id_out) {
    // Get folderpath
    size_t pos = filepath.rfind("/");
    if(pos != std::string::npos) {
        std::string folderpath = filepath.substr(0, pos);
        utils::CheckUrlAndRemoveTrailingSlash(folderpath);
        FolderHandler fh(file_manager_);
        Folder folder;
        if(fh.GetFolderByAbsolutePath(folderpath, folder)){
            if(!folder.folder_post_id().empty()) {
                id_out = folder.folder_post_id();
                return true;
            }
        }
    }
    return false;
}

} // namespace

