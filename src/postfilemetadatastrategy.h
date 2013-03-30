#ifndef POSTFILEMETADATASTRATEGY_H_
#define POSTFILEMETADATASTRATEGY_H_
#pragma once

#include <string>
#include "httpstrategy.h"

namespace attic { 

class FileManager;
class CredentialsManager;
class FileInfo;

class PostFileMetadataStrategy : public HttpStrategyInterface {
    int SendFilePost( FileInfo* fi, const std::string& filepath);
    FileInfo* RetrieveFileInfo(const std::string& filepath);
public:
    PostFileMetadataStrategy();
    ~PostFileMetadataStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager,
                Response& out);
};

}//namespace
#endif
