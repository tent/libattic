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

    bool CreateNewFile(const std::string& filepath, 
                       const std::string& master_key,
                       FileInfo& out);

    bool UpdateFilePostId(const std::string& filepath, const std::string& post_id);

    bool EncryptFileKey(const std::string& filepath, 
                        const std::string& file_key,
                        const std::string& master_key);

    bool EncryptFileKey(FileInfo& fi, const std::string& master_key);

    bool EncryptFileKey(const std::string& file_key,
                        const std::string& file_iv,
                        const std::string& master_key,
                        std::string& encrypted_out);

private:
    FileManager* file_manager_;
};

} //namespace
#endif

