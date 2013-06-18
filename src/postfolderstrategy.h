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
    int CreateFolderPost(Folder& folder, std::string& id_out);
    int RetrieveFolderPost(const std::string& post_id, FolderPost& out);
    bool UpdateFolderPost(Folder& folder, const std::string post_id);
    int PostFolderPost(const std::string& post_id, FolderPost& fp);
public:
    PostFolderStrategy() {}
    ~PostFolderStrategy() {}

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);

};

}//namespace
#endif

