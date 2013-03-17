#ifndef SOFTDELETESTRATEGY_H_
#define SOFTDELETESTRATEGY_H_
#pragma once

#include "httpstrategy.h"

class SoftDeleteStrategy : public HttpStrategyInterface {

    FileInfo* RetrieveFileInfo(const std::string& filepath);
public:
    SoftDeleteStrategy();
    ~SoftDeleteStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager,
                const std::string& entityApiRoot, 
                const std::string& filepath, 
                Response& out);
};

#endif

