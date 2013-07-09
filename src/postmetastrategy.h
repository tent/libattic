#ifndef POSTMETASTRATEGY_H_
#define POSTMETASTRATEGY_H_
#pragma once

#include <string>
#include "httpstrategy.h"
/* This strategy will initialize a file's meta data and nothing more. */
namespace attic {

class FileManager;
class CredentialsManager;

class PostMetaStrategy : public HttpStrategyInterface{ 
    int CreateFileEntry(const std::string& filepath, FileInfo& out);
    bool RetrieveFolderPostId(const std::string& filepath, std::string& id_out);
public:
    PostMetaStrategy() {}
    ~PostMetaStrategy() {}

    int Execute(FileManager* fm, CredentialsManager* cm);
};

} // namespace

#endif

