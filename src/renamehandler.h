#ifndef RENAMEHANDLER_H_
#define RENAMEHANDLER_H_
#pragma once

#include <vector>
#include <string>
#include "filepost.h"

namespace attic {
    
class FileManager;

class RenameHandler {
public:
    RenameHandler(FileManager* fi);
    ~RenameHandler();
    
    int RenameFileLocalCache(const std::string& old_filepath,
                             const std::string& new_name,
                             std::string& new_filepath);

    void UpdateFileMetaPost(FilePost& fp, 
                            const FileInfo& fi, 
                            FilePost& out);
   
    bool CheckForRename(FilePost& fp);
private:
    FileManager* file_manager_;
};

} //namespace
#endif
