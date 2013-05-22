#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_
#pragma once

#include <string>
#include "fileinfo.h"
#include "filemanager.h"

namespace attic { 

class FileHandler {
public:
    FileHandler(FileManager* fm);
    ~FileHandler();

    bool DoesFileExist(const std::string& filepath);
    bool RetrieveFileInfo(const std::string& filepath, FileInfo& out);
    bool CreateNewFile(const std::string& filepath, FileInfo& out);
    bool UpdateFilePostId(const std::string& filepath, const std::string& post_id);

private:
    FileManager* file_manager_;
};

} //namespace
#endif

