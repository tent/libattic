#include "softdeletestrategy.h"

namespace attic { 

SoftDeleteStrategy::SoftDeleteStrategy() {}
SoftDeleteStrategy::~SoftDeleteStrategy() {}

int SoftDeleteStrategy::Execute(FileManager* pFileManager,
                                CredentialsManager* pCredentialsManager){
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);

    post_path_ = GetConfigValue("post_path");
    std::string filepath = GetConfigValue("filepath");

    FileInfo* fi = RetrieveFileInfo(filepath);
    if(fi)
        MarkFileDeleted(fi);
    else 
        status = ret::A_FAIL_INVALID_FILE_INFO;

    return status;
}

void SoftDeleteStrategy::MarkFileDeleted(FileInfo* fi) {
    int status = ret::A_OK;
    std::string filepath = fi->filepath();
    fi->set_deleted(1);
    file_manager_->SetFileDeleted(filepath, true);
}

FileInfo* SoftDeleteStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = file_manager_->GetFileInfo(filepath);
    if(!fi)
        fi = file_manager_->CreateFileInfo();
    return fi;
}

}//namespace
