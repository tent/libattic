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

    std::cout<<" posting meta data strategy ... " << filepath << std::endl;

    if(ValidMasterKey()) {
        std::cout<<" valid master key " << std::endl;
        FileHandler fh(file_manager_);
        if(!fh.DoesFileExist(filepath)) {
            std::cout<<" does file exist ... yes " << std::endl;
            // File doesn't exist yet, create meta post
            FileInfo fi;
            status = CreateFileEntry(filepath, fi);
            std::cout<<" create file entry : " << status << std::endl;
            if (status == ret::A_OK)
                status = CreateFileMetaPost(filepath, fi);
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
    return status;
}

int PostMetaStrategy::CreateFileEntry(const std::string& filepath, FileInfo& out) {
    int status = ret::A_OK;
    std::string mk;
    GetMasterKey(mk);
    FileHandler fh(file_manager_);
    if(!fh.CreateNewFile(filepath, mk, out))
        status = ret::A_FAIL_CREATE_FILE_INFO;
    return status;
}

int PostMetaStrategy::CreateFileMetaPost(const std::string& filepath, FileInfo& fi) {
    int status = ret::A_OK;
    FilePost fp(fi);
    fp.set_fragment(cnst::g_transit_fragment);
    PostHandler<FilePost> ph(access_token_);
    Response response;
    if(ph.Post(posts_feed_, NULL, fp, response) == ret::A_OK) {
        FilePost post;
        jsn::DeserializeObject(&post, response.body);
        fi.set_post_id(post.id());
        FileHandler fh(file_manager_);
        fh.UpdateFilePostId(fi.filepath(), post.id());
    }
    return status;
}

bool PostMetaStrategy::RetrieveFolderPostId(const std::string& filepath, std::string& id_out) {
    // Get folderpath
    std::cout<<" incoming filepath : " << filepath << std::endl;
    size_t pos = filepath.rfind("/");
    if(pos != std::string::npos) {
        std::string folderpath = filepath.substr(0, pos);
        utils::CheckUrlAndRemoveTrailingSlash(folderpath);
        FolderHandler fh(file_manager_);
        Folder folder;
        std::cout<<" getting folder : " << folderpath << std::endl;
        if(fh.GetFolder(folderpath, folder)){
            if(!folder.folder_post_id().empty()) {
                id_out = folder.folder_post_id();
                return true;
            }
        }
    }
    return false;
}

bool PostMetaStrategy::ValidMasterKey() {
    std::string mk;
    GetMasterKey(mk);
    if(mk.empty()) {
        std::string error = "Invalid master key, it is empty!";
        log::LogString("019288--1908-50", error);
        return false;
    }
    return true;
}

void PostMetaStrategy::GetMasterKey(std::string& out) {
    MasterKey mKey;
    credentials_manager_->GetMasterKeyCopy(mKey);
    mKey.GetMasterKey(out);
}

} // namespace

