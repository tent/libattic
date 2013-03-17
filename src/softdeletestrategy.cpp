#include "softdeletestrategy.h"

SoftDeleteStrategy::SoftDeleteStrategy() {}
SoftDeleteStrategy::~SoftDeleteStrategy() {}

int SoftDeleteStrategy::Execute(FileManager* pFileManager,
                                CredentialsManager* pCredentialsManager,
                                const std::string& entityApiRoot, 
                                const std::string& filepath, 
                                Response& out)
{
    int status = ret::A_OK;
    m_entityApiRoot = entityApiRoot;
    m_pFileManager = pFileManager;
    m_pCredentialsManager = pCredentialsManager;
    if(!m_pFileManager) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    if(!m_pCredentialsManager) return ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    m_pCredentialsManager->GetAccessTokenCopy(m_At);


    return status;
}

FileInfo* SoftDeleteStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = m_pFileManager->GetFileInfo(filepath);
    if(!fi)
        fi = m_pFileManager->CreateFileInfo();
    return fi;
}
