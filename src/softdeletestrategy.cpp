#include "softdeletestrategy.h"

namespace attic { 

SoftDeleteStrategy::SoftDeleteStrategy() {}
SoftDeleteStrategy::~SoftDeleteStrategy() {}

int SoftDeleteStrategy::Execute(FileManager* pFileManager,
                                CredentialsManager* pCredentialsManager,
                                Response& out)
{
    int status = ret::A_OK;
    m_pFileManager = pFileManager;
    m_pCredentialsManager = pCredentialsManager;
    if(!m_pFileManager) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    if(!m_pCredentialsManager) return ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    m_pCredentialsManager->GetAccessTokenCopy(m_At);

    m_entityApiRoot = GetConfigValue("api_root");
    std::string filepath = GetConfigValue("filepath");

    FileInfo* fi = RetrieveFileInfo(filepath);
    if(fi) {
        MarkFileDeleted(fi);
    }
    else {
        status = ret::A_FAIL_INVALID_FILE_INFO;
    }

    return status;
}

void SoftDeleteStrategy::MarkFileDeleted(FileInfo* fi) {
    int status = ret::A_OK;
    std::string filepath;
    fi->GetFilepath(filepath);
    fi->SetDeleted(1);
    m_pFileManager->SetFileDeleted(filepath, true);
}

FileInfo* SoftDeleteStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = m_pFileManager->GetFileInfo(filepath);
    if(!fi)
        fi = m_pFileManager->CreateFileInfo();
    return fi;
}

}//namespace
