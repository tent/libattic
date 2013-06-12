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
    typedef std::map<std::string, ChunkInfo> ChunkMap; // key : chunk: name // value : chunk

    FileInfo(const std::string& filename,
             const std::string& filepath,
             const std::string& postid);
    FileInfo();  
    ~FileInfo(); 

    bool InitializeFile(const std::string &filepath); // Init a file

    int PushChunkBack(ChunkInfo& chunk);
    ChunkInfo* GetChunkInfo(const std::string& chunkname);
    ChunkMap* GetChunkInfoList() { return &chunks_; }

    bool DoesChunkExist(const std::string& chunk_name);

    void GetSerializedChunkData(std::string& out) const;
    bool LoadSerializedChunkData(const std::string& data);
    bool HasEncryptedKey();
    bool IsValid() { return !filename_.empty(); }
    
    // Accessors & Mutators
    const std::string& filename() const             { return filename_; }
    const std::string& filepath() const             { return filepath_; }
    const std::string& post_id() const              { return post_id_; }

    const Credentials& file_credentials() const     { return file_credentials_; }
    const std::string& file_credentials_key() const { return file_credentials_.key(); }
    const std::string& file_credentials_iv() const  { return file_credentials_.iv(); }
    const std::string& encrypted_key() const        { return encrypted_key_; }
    const std::string& post_version() const         { return post_version_; }
    const std::string& folder_post_id() const       { return folder_post_id_; }
    const std::string& plaintext_mac() const        { return plaintext_mac_; }

    bool deleted() const                            { return deleted_; }
    unsigned int chunk_count() const                { return chunk_count_; }
    unsigned int file_size() const                  { return file_size_; }

    void set_filename(const std::string &filename)          { filename_ = filename; }
    void set_filepath(const std::string &filepath)          { filepath_ = filepath; }
    void set_chunks(const ChunkMap& chunk_map)              { chunks_ = chunk_map; }
    void set_chunk_count(const std::string& count)          { chunk_count_ = atoi(count.c_str()); }
    void set_chunk_count(const unsigned int unChunkCount)   { chunk_count_ = unChunkCount; }
    void set_file_size(const std::string& file)             { file_size_ = atoi(file.c_str()); }
    void set_file_size(const unsigned int unFileSize)       { file_size_ = unFileSize; }

    void set_post_id(const std::string &id)                 { post_id_ = id; }
    void set_post_version(const std::string& version)       { post_version_ = version; }


    void set_file_credentials(const Credentials& cred);
    void set_file_credentials_key(const std::string &key)   { file_credentials_.set_key(key); }
    void set_file_credentials_iv(const std::string &iv)     { file_credentials_.set_iv(iv); }
    void set_encrypted_key(const std::string& key)          { encrypted_key_ = key; } 
    void set_folder_post_id(const std::string& id)          { folder_post_id_ = id; }
    //void set_deleted(const std::string& deleted)          { deleted_ = atoi(deleted.c_str()); }
    void set_deleted(const bool deleted)                    { deleted_ = deleted; }

    void set_plaintext_mac(const std::string& mac) { 
        // avoid cow
        plaintext_mac_.append(mac.c_str(), mac.size());
    }

private:    
    ChunkMap        chunks_;
    Credentials     file_credentials_; // File Specific credentials 
                                       // Key is used to encrypt chunks, an iv is specific to chunk
                                       // Iv is used to encrypt this key
    std::string     encrypted_key_;
    std::string     folder_post_id_;    // folder post id in the manifest

    std::string     filename_;          // File within directory
    std::string     filepath_;          // Directory
    std::string     post_id_;           // Id of the post the file is potentially attached to 

    std::string     encryption_algorithm_; // Algorithm used to encrypt chunks
    std::string     hashing_algorithm_;
    std::string     compression_algorithm_;
    std::string     compression_level_;
    std::string     plaintext_mac_;         // hmac of entire file 32 bytes

    std::string     post_version_;      // Version of the post the file is attached to
    unsigned int    chunk_count_;       // depricated
    unsigned int    file_size_;         // Filesize, not compressed
    bool            deleted_;           // Is the file deleted? // soft delete
};

}//namespace
#endif

