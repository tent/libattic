#ifndef SOFTDELETESTRATEGY_H_
#define SOFTDELETESTRATEGY_H_
#pragma once

#include <string>

#include "httpstrategy.h"
#include "fileinfo.h"
#include "filepost.h"

namespace attic { 

class SoftDeleteStrategy : public HttpStrategyInterface {
    void MarkFileDeleted(FileInfo* fi);
    int UpdateFilePost(FileInfo* fi);
    int RetrieveFilePost(const std::string& post_id, FilePost& out);

    int PostFilePost(const std::string& post_id, FilePost& fp);
public:
    SoftDeleteStrategy() {}
    ~SoftDeleteStrategy() {}

    int Execute(FileManager* file_manager,
                CredentialsManager* credentials_manager_);
};

}//namespace
#endif

