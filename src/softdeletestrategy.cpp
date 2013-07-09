#include "softdeletestrategy.h"

#include "utils.h"
#include "posthandler.h"

namespace attic { 

int SoftDeleteStrategy::Execute(FileManager* file_manager,
                                CredentialsManager* credentials_manager_){
    int status = ret::A_OK;
    status = InitInstance(file_manager, credentials_manager_);
    std::cout<<" del " << std::endl;

    post_path_ = GetConfigValue("post_path");
    std::string filepath = GetConfigValue("filepath");

    FileInfo fi;
    if(file_manager_->GetFileInfo(filepath, fi)) {
    std::cout<<" del " << std::endl;
        MarkFileDeleted(&fi);
        status = UpdateFilePost(&fi);
    }
    else 
        status = ret::A_FAIL_INVALID_FILE_INFO;

    return status;
}

void SoftDeleteStrategy::MarkFileDeleted(FileInfo* fi) {
    int status = ret::A_OK;
    fi->set_deleted(true);
    file_manager_->SetFileDeleted(fi->post_id(), true);
}

int SoftDeleteStrategy::UpdateFilePost(FileInfo* fi) {
    int status = ret::A_OK;
    std::string postid = fi->post_id();
    FilePost fp;
    status = RetrieveFilePost(postid, fp);
    if(status == ret::A_OK) {
        fp.set_fragment(cnst::g_deleted_fragment);
        status = PostFilePost(postid, fp);
    }
    return status;
}

int SoftDeleteStrategy::RetrieveFilePost(const std::string& post_id, FilePost& out) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string posturl;
        utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

        PostHandler<FilePost> ph(access_token_);
        status = ph.Get(posturl, NULL, out);
        if(status != ret::A_OK)
            log::LogHttpResponse("175kjas", ph.response());
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

int SoftDeleteStrategy::PostFilePost(const std::string& post_id, FilePost& fp) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string posturl;
        utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

        PostHandler<FilePost> ph(access_token_);
        status = ph.Put(posturl, NULL, fp);

        if(status != ret::A_OK);
            log::LogHttpResponse("192151mm", ph.response());
    }
    else { 
        status = ret::A_FAIL_INVALID_POST_ID;
    }

    return status;
}

}//namespace
