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


    bool RenameFolderLocalCache(const std::string& folder_post_id,
                                const std::string& new_foldername);
    int UpdateFileMetaData(std::string& old_filepath,
                           std::string& new_filepath);
    void UpdateFileMetaPost(FilePost& fp, 
                            const FileInfo& fi, 
                            FilePost& out);

    void UpdateFolderMetaPost(FolderPost& fp,
                              const Folder& folder,
                              FolderPost& out);
   
    bool CheckForRename(FolderPost& fp);

    bool RenameFileLocalCacheAbsolutePath(const std::string& absolute_path, 
                                          const std::string& new_filename);
    bool RenameFileLocalCache(const std::string& post_id, const std::string& new_filename);
private:
    FileManager* file_manager_;
};

} //namespace
#endif

