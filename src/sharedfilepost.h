#ifndef SHAREDFILEPOST_H_
#define SHAREDFILEPOST_H_
#pragma once

#include <string>
#include "post.h"
#include "filepost.h"

namespace attic {

class SharedFilePost : public Post { 
    void SerializeChunkData(Json::Value& root);
    void DeserializeChunkData(Json::Value& root);
public:
    SharedFilePost();
    ~SharedFilePost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    FileInfo::ChunkMap* chunk_data() { return &chunk_data_; }

    unsigned int file_size() const              { return file_size_; }
    const std::string& filename() const         { return filename_; }
    const std::string& encrypted_key() const    { return encrypted_key_; }
    const std::string& iv() const               { return iv_; }
    const std::string& nonce() const            { return nonce_; }
    const std::string& owner() const            { return owner_; }

    void set_encrypted_key(const std::string& key) { encrypted_key_ = key; }
    void set_iv(const std::string& iv)             { iv_ = iv; }
    void set_nonce(const std::string& nonce)       { nonce_ = nonce; }
    void set_owner(const std::string& owner)       { owner_ = owner; }
    void set_chunk_data(FileInfo::ChunkMap& cd)    { chunk_data_ = cd; }
    void set_filename(const std::string& filename) { filename_ = filename; }
private:
    FileInfo::ChunkMap chunk_data_;

    // File info specific things
    std::string filename_;
    std::string plaintext_hash_;
    std::string deleted_;

    std::string encrypted_key_;
    std::string iv_; // file iv
    std::string nonce_; // iv used for encrypted key

    std::string owner_; // entity url of owner

    unsigned int file_size_;
};


}//namespace 
#endif

