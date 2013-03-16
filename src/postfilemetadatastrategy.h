#ifndef POSTFILEMETADATASTRATEGY_H_
#define POSTFILEMETADATASTRATEGY_H_
#pragma once

#include <string>
#include "httpstrategy.h"

class FileManager;
class CredentialsManager;
class FileInfo;

class PostFileMetadataStrategy : public HttpStrategyInterface {
    int SendAtticPost( FileInfo* fi, const std::string& filepath);
    FileInfo* RetrieveFileInfo(const std::string& filepath);
public:
    PostFileMetadataStrategy();
    ~PostFileMetadataStrategy();

    void Execute(FileManager* pFileManager,
                 CredentialsManager* pCredentialsManager,
                 const std::string& entityApiRoot, 
                 const std::string& filepath, 
                 Response& out);

private:
    CredentialsManager*     m_pCredentialsManager;
    FileManager*            m_pFileManager;

    AccessToken             m_At;
    std::string             m_entityApiRoot;
};

#endif
