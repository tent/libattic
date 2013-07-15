#ifndef SHAREDFILEPOST_H_
#define SHAREDFILEPOST_H_
#pragma once

#include <string>
#include "post.h"
#include "filepost.h"

namespace attic {

class SharedFilePost : public Post { 
public:
    SharedFilePost();
    ~SharedFilePost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    FileInfo::ChunkMap* chunk_data() { return &chunk_data_; }

    unsigned int file_size() const              { return file_size_; }
    const std::string& cargo() const            { return cargo_; }
    const std::string& filename() const         { return filename_; }
    const std::string& encrypted_key() const    { return encrypted_key_; }
    const std::string& iv() const               { return iv_; }
    const std::string& owner() const            { return owner_; }

    void set_encrypted_key(const std::string& key);
    void set_iv(const std::string& iv);
    void set_cargo_(const std::string& c)       { cargo_ = c; }
private:
    FileInfo::ChunkMap chunk_data_;

    std::string cargo_; // Encrypted filename and filepath

    std::string filename_;

    std::string encrypted_key_;
    std::string iv_;

    std::string owner_; // entity url of owner

    unsigned int file_size_;
};


}//namespace 
#endif

