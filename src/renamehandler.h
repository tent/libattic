#ifndef RENAMEHANDLER_H_
#define RENAMEHANDLER_H_
#pragma once

#include <vector>
#include <string>
#include "filepost.h"
#include "folderpost.h"

namespace attic {
    
class FileManager;

class RenameHandler {
public:
    RenameHandler(FileManager* fi);
    ~RenameHandler();
    
    int RenameFileLocalCache(const std::string& old_filepath,
                             const std::string& new_filepath);

    void UpdateFileMetaPost(FilePost& fp, 
                            const FileInfo& fi, 
                            FilePost& out);

    void UpdateFolderMetaPost(FolderPost& fp,
                              const Folder& folder,
                              FolderPost& out);

    int RenameFolderLocalCache(const std::string& old_folderpath,
                               const std::string& new_folderpath);
   
    bool CheckForRename(FilePost& fp);
    bool CheckForRename(FolderPost& fp);
private:
    FileManager* file_manager_;
};

} //namespace
#endif

