#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_
#pragma once

#include <string>
#include "fileinfo.h"
#include "filemanager.h"

class FileHandler {
public:
    FileHandler(FileManager* fm);
    ~FileHandler();

    bool DoesFileExist(const std::string& filepath);
    bool RetrieveFileInfo(const std::string& filepath, FileInfo& out);
    bool CreateNewFile(const std::string& filepath, FileInfo& out);

private:
    FileManager* file_manager_;
};

#endif

