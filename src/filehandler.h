#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_
#pragma once

#include <string>
#include "fileinfo.h"
#include "filemanager.h"
#include "filepost.h"
#include "folderpost.h"
#include "accesstoken.h"

namespace attic { 

class FileHandler {
    bool CreateNewFileMetaPost(FileInfo& fi, 
                               const std::string& master_key, 
                               const std::string& posts_feed,
                               const AccessToken& at,
                               FilePost& out);
public:
    FileHandler(FileManager* fm);
    ~FileHandler();

    bool DoesFileExist(const std::string& filepath);

    bool RetrieveFileInfo(const std::string& filepath, FileInfo& out);
    bool RetrieveFileInfoById(const std::string& post_id, FileInfo& out);

    bool CreateNewFile(const std::string& filepath, 
                       const std::string& master_key,
                       const std::string& posts_feed,
                       const AccessToken& at,
                       FileInfo& out);

    bool GetCanonicalFilepath(const std::string& filepath, std::string& out);

    bool UpdateFileInfo(FileInfo& fi);

    bool UpdateFilepath(const std::string& post_id, const std::string& new_folder_post_id);

    bool UpdateFileSize(const std::string& filepath, const std::string& size);
    bool UpdateChunkMap(const std::string& filepath, FileInfo::ChunkMap& map);
    bool UpdatePostVersion(const std::string& post_id, const std::string& version);
    bool UpdateFolderEntry(FolderPost& fp);
    // Utils
    void PrepareFilePost(FileInfo& fi,
                         const std::string& master_key,
                         FilePost& out); 
    void DeserializeIntoFileInfo(FilePost& fp, 
                                 const std::string& master_key,
                                 FileInfo& out);
    bool GetTemporaryFilepath(FileInfo& fi, std::string& path_out);

    // Crypto
    void PrepareCargo(FileInfo& fi, 
                      const std::string& master_key, 
                      std::string& cargo_out);

    void UnpackCargo(FilePost& fp, 
                     const std::string& master_key,
                     Cargo& open_cargo);

    bool RollFileMac(const std::string& filepath, std::string& out);
    bool EncryptFileKey(const std::string& filepath, 
                        const std::string& file_key,
                        const std::string& master_key);

    bool EncryptFileKey(FileInfo& fi, const std::string& master_key);

    bool EncryptFileKey(const std::string& file_key,
                        const std::string& file_iv,
                        const std::string& master_key,
                        std::string& encrypted_out);

    bool DecryptFileKey(const std::string& encrypted_key,
                        const std::string& file_iv,
                        const std::string& master_key,
                        std::string& decrypted_out);
    bool ExtractFileCredetials(const FilePost& fp,
                               const std::string& master_key,
                               Credentials& out);
private:
    FileManager* file_manager_;
};

} //namespace
#endif

