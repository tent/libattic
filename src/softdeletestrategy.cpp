#include "softdeletestrategy.h"

#include "netlib.h"
#include "utils.h"

namespace attic { 

int SoftDeleteStrategy::Execute(FileManager* file_manager,
                                CredentialsManager* credentials_manager_){
    int status = ret::A_OK;
    status = InitInstance(file_manager, credentials_manager_);
    std::cout<<" del " << std::endl;

    post_path_ = GetConfigValue("post_path");
    std::string filepath = GetConfigValue("filepath");

    FileInfo* fi = RetrieveFileInfo(filepath);
    if(fi) { 
    std::cout<<" del " << std::endl;
        MarkFileDeleted(fi);
        status = UpdateFilePost(fi);
    }
    else 
        status = ret::A_FAIL_INVALID_FILE_INFO;

    return status;
}

void SoftDeleteStrategy::MarkFileDeleted(FileInfo* fi) {
    int status = ret::A_OK;
    std::string filepath = fi->filepath();
    fi->set_deleted(true);
    file_manager_->SetFileDeleted(filepath, true);
}

FileInfo* SoftDeleteStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = file_manager_->GetFileInfo(filepath);
    if(!fi)
        fi = file_manager_->CreateFileInfo();
    return fi;
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

        std::cout<<" POST URL : " << posturl << std::endl;

        Response resp;
        netlib::HttpGet(posturl,
                        NULL,
                        &access_token_,
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

int SoftDeleteStrategy::PostFilePost(const std::string& post_id, FilePost& fp) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string posturl;
        utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
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
                         &access_token_,
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

}//namespace
