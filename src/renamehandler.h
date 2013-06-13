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

    int RenameFolderLocalCache(const std::string& old_folderpath,
                               const std::string& new_folderpath);

    void UpdateFileMetaPost(FilePost& fp, 
                            const FileInfo& fi, 
                            FilePost& out);

    void UpdateFolderMetaPost(FolderPost& fp,
                              const Folder& folder,
                              FolderPost& out);
   
    bool CheckForRename(FileInfo& fi, const std::string& post_id);
    bool CheckForRename(FolderPost& fp);
private:
    FileManager* file_manager_;
};

} //namespace
#endif

