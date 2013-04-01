#ifndef FILEINFO_H_
#define FILEINFO_H_
#pragma once

#include <string>
#include <map>
#include <vector>

#include "crypto.h"
#include "chunkinfo.h"

namespace attic { 

class FileInfo {
    void ExtractFilename(const std::string &filepath, std::string &out);
public:
    typedef std::map<std::string, ChunkInfo> ChunkMap;

    FileInfo( const std::string& filename,
              const std::string& filepath,
              const std::string& postid,
              const std::string& chunkpostid);

    FileInfo();  
    ~FileInfo(); 

    // Init a file
    bool InitializeFile(const std::string &filepath);

    bool IsValid() { return !filename_.empty(); }
    
    void SetFilename(const std::string &filename)     { filename_ = filename; }
    void SetFilepath(const std::string &filepath)     { filepath_ = filepath; }

    void SetChunkName(const std::string &chunkname)     { m_ChunkName = chunkname; }
    void SetChunkCount(const std::string& count)        { chunk_count_ = atoi(count.c_str()); }
    void SetChunkCount(const unsigned int unChunkCount) { chunk_count_ = unChunkCount; }

    void SetFileSize(const std::string& file)           { file_size_ = atoi(file.c_str()); }
    void SetFileSize(const unsigned int unFileSize)     { file_size_ = unFileSize; }

    void SetPostID(const std::string &szID)         { post_id_ = szID; }
    void SetChunkPostID(const std::string &szID)    {chunk_post_id_ = szID; }
    void SetPostVersion(const std::string& version) { post_version_ = version; }

    void SetFileCredentials(const Credentials& cred)    { file_credentials_ = cred; }
    void SetFileKey(const std::string &key)             { file_credentials_.set_key(key); }
    void SetEncryptedKey(const std::string& key)        { encrypted_key_ = key; } 
    void SetIv(const std::string &iv)                   { file_credentials_.set_iv(iv); }
    void SetDeleted(const std::string& deleted)         { deleted_ = atoi(deleted.c_str()); }
    void SetDeleted(const int deleted)                 { deleted_ = deleted; }

    void GetFilename(std::string &out) const    { out = filename_; }
    void GetFilepath(std::string &out) const    { out = filepath_; }
    void GetPostID(std::string &out) const      { out = post_id_; }
    void GetChunkPostID(std::string &out) const { out =chunk_post_id_; }
    void GetChunkName(std::string& out) const   { out = m_ChunkName; }

    void GetFileCredentials(Credentials& cred) const    { cred = file_credentials_; }
    void GetFileKey(std::string &out) const             { out = file_credentials_.key(); }
    void GetEncryptedKey(std::string& out) const        { out = encrypted_key_; }
    void GetIv(std::string &out) const                  { out = file_credentials_.iv(); }
    int GetDeleted() const                              { return deleted_; }

    unsigned int GetChunkCount() const      { return chunk_count_; }
    unsigned int GetFileSize() const        { return file_size_; }

    Credentials GetCredentialsCopy() const  { return file_credentials_; }

    const std::string& GetPostVersion() const { return post_version_ ; }

    int PushChunkBack(ChunkInfo& Chunk);
    ChunkInfo* GetChunkInfo(const std::string& chunkname);

    ChunkMap* GetChunkInfoList() { return &chunks_; }

    void GetSerializedChunkData(std::string& out) const;
    bool LoadSerializedChunkData(const std::string& data);
    bool LoadSerializedChunkPost(const std::string& data);

    bool HasEncryptedKey();

private:    
    ChunkMap        chunks_;
    Credentials     file_credentials_; // File Specific credentials 
                                       // Key is used to encrypt chunks, an iv is specific to chunk
                                       // Iv is used to encrypt this key
    std::string     encrypted_key_;

    std::string     filename_;   // File within directory
    std::string     filepath_;   // Directory
    std::string     m_ChunkName;                                                    // Depricated
    std::string     post_id_; // Id of the post the file is potentially attached to
    std::string     chunk_post_id_; // Id of the chunk post

    std::string     encryption_algorithm_; // Algorithm used to encrypt chunks
    std::string     hashing_algorithm_;
    std::string     compression_algorithm_;
    std::string     compression_level_;

    std::string     post_version_; // Version of the post the file is attached to
    unsigned int    chunk_count_; // depricated
    unsigned int    file_size_;   // Filesize, not compressed
    int    deleted_; // Is the file deleted? // soft delete
};

}//namespace
#endif

