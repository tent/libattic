#ifndef SOFTDELETESTRATEGY_H_
#define SOFTDELETESTRATEGY_H_
#pragma once

#include "httpstrategy.h"

namespace attic { 

class SoftDeleteStrategy : public HttpStrategyInterface {
    void MarkFileDeleted(FileInfo* fi);
    FileInfo* RetrieveFileInfo(const std::string& filepath);
public:
    SoftDeleteStrategy();
    ~SoftDeleteStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);
};

}//namespace
#endif

