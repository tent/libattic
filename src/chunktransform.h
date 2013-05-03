#ifndef CHUNKTRANSFORM_H_
#define CHUNKTRASNFORM_H_
#pragma once

#include <string>

class ChunkTransform {
    void GenerateChunkName();
    void Compress(const std::string& in, std::string& out);
    void Encrypt(const std::string& in, std::string& out);
    void Encode(const std::string& in, std::string& out);
public:
    ChunkTransform(const std::string& chunk, const std::string& file_key);
    ~ChunkTransform();

    void Transform();

    const std::string& file_key() const         { return file_key_; }
    const std::string& name() const             { return name_; }
    const std::string& data() const             { return data_; }
    const std::string& finalized_data() const   { return finalized_data_; }
    const std::string& chunk_iv() const         { return chunk_iv_; }
private:
    // meta data
    std::string chunk_iv_;
    std::string file_key_;
    std::string name_;
    std::string plaintext_hash_;
    std::string ciphertext_hash_;

    // core data
    std::string data_;
    std::string finalized_data_;
};

#endif

