#ifndef RENAMESTRATEGY_H_
#define RENAMESTRATEGY_H_
#pragma once

#include <string>
#include "httpstrategy.h"
#include "filepost.h"
#include "folderpost.h"

namespace attic { 

class FileManager;
class FileInfo;

class RenameStrategy : public HttpStrategyInterface {
    int RenameFile();
    int RenameFolder();
    FileInfo* RetrieveFileInfo(const std::string& filepath);
    int RetrieveFolderPost(const std::string& post_id, FolderPost& fp);
    int RetrieveFilePost(const std::string& post_id, FilePost& fp);
    int UpdateFileMetaPost(const std::string& post_id, 
                           const std::string& filename,
                           const std::string& relative_path);

    int UpdateFolderMetaPost(const std::string& folderpath, Folder& folder);
public:
    RenameStrategy();
    ~RenameStrategy();

    int Execute(FileManager* pFileManager, CredentialsManager* pCredentialsManager);
};

} //namespace
#endif

