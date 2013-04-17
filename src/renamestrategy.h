#ifndef RENAMESTRATEGY_H_
#define RENAMESTRATEGY_H_
#pragma once

#include <string>
#include "httpstrategy.h"
#include "filepost.h"

namespace attic { 

class FileManager;
class FileInfo;

class RenameStrategy : public HttpStrategyInterface {
    FileInfo* RetrieveFileInfo(const std::string& filepath);
    int RetrieveFilePost(const std::string& post_id, FilePost& fp);
public:
    RenameStrategy();
    ~RenameStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);
};

} //namespace
#endif

