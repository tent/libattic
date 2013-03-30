#include "syncstrategy.h"

namespace attic { 

SyncStrategy::SyncStrategy() {}
SyncStrategy::~SyncStrategy() {}

int SyncStrategy::Execute(FileManager* pFileManager,
                          CredentialsManager* pCredentialsManager,
                          const std::string& entityApiRoot, 
                          const std::string& filepath, 
                          Response& out) 
{
    int status = ret::A_OK;
    post_path_ = entityApiRoot;
    file_manager_ = pFileManager;
    credentials_manager_ = pCredentialsManager;
    if(!file_manager_) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    if(!credentials_manager_) return ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    credentials_manager_->GetAccessTokenCopy(access_token_);

    // TODO :: this, later


    return status;
}

}//namespace

