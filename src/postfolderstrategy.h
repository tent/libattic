#ifndef POSTFOLDERSTRATEGY_H_
#define POSTFOLDERSTRATEGY_H_
#pragma once

#include <string>
#include <vector>
#include "httpstrategy.h"

namespace attic { 

class FileManager;
class CredentialsManager;

class PostFolderStrategy : public HttpStrategyInterface {
    void ExtractDirectories(const std::string& filepath, std::vector<std::string>& directories);
public:
    PostFolderStrategy() {}
    ~PostFolderStrategy() {}

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);

};

}//namespace
#endif

