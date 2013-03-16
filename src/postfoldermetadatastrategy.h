#ifndef POSTFOLDERMETADATASTRATEGY_H_
#define POSTFOLDERMETADATASTRATEGY_H_
#pragma once

#include <string>
#include "httpstrategy.h"

class FileManager;
class CredentialsManager;
class FileInfo;

class PostFolderMetadataStrategy : public HttpStrategyInterface {
    FileInfo* RetrieveFileInfo(const std::string& filepath);
    int SendFolderPost(const FileInfo* fi, Response& out);
public:
    PostFolderMetadataStrategy();
    ~PostFolderMetadataStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager,
                const std::string& entityApiRoot, 
                const std::string& filepath, 
                Response& out);
};

#endif

