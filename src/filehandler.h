#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_
#pragma once

#include <string>
#include "fileinfo.h"
#include "filemanager.h"
#include "filepost.h"
#include "folderpost.h"

namespace attic { 

class FileHandler {
public:
    FileHandler(FileManager* fm);
    ~FileHandler();

    bool DoesFileExist(const std::string& filepath);

    bool RetrieveFileInfo(const std::string& filepath, FileInfo& out);
    bool RetrieveFileInfoById(const std::string& post_id, FileInfo& out);

    bool CreateNewFile(const std::string& filepath, 
                       const std::string& master_key,
                       FileInfo& out);

    bool GetCanonicalFilepath(const std::string& filepath, std::string& out);

    bool UpdateFileInfo(FileInfo& fi);
    bool UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath);
    bool UpdateFilePostId(const std::string& filepath, const std::string& post_id);
    bool UpdateChunkCount(const std::string& filepath, const std::string& count);
    bool UpdateFileSize(const std::string& filepath, const std::string& size);
    bool UpdateChunkMap(const std::string& filepath, FileInfo::ChunkMap& map);
    bool UpdatePostVersion(const std::string& filepath, const std::string& version);
    bool UpdateFolderEntry(FolderPost& fp);
    // Utils
    void DeserializeIntoFileInfo(FilePost& fp, FileInfo& out);
    bool GetTemporaryFilepath(FileInfo& fi, std::string& path_out);

    // Crypto
    bool EncryptFileKey(const std::string& filepath, 
                        const std::string& file_key,
                        const std::string& master_key);

    bool EncryptFileKey(FileInfo& fi, const std::string& master_key);

    bool EncryptFileKey(const std::string& file_key,
                        const std::string& file_iv,
                        const std::string& master_key,
                        std::string& encrypted_out);

    bool ExtractFileCredetials(const FilePost& fp,
                               const std::string& master_key,
                               Credentials& out);

private:
    FileManager* file_manager_;
};

} //namespace
#endif

