#ifndef CHUNKINFO_H_
#define CHUNKINFO_H_
#pragma once

#include <iostream>
#include <string>

#include "jsonserializable.h"

namespace attic {

class ChunkInfo : public JsonSerializable {
public:
    ChunkInfo();
    ChunkInfo(const std::string& chunkname, const std::string& checksum);
    ~ChunkInfo();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    const std::string& chunk_name() const       { return chunk_name_; }
    const std::string& checksum() const         { return checksum_; }
    const std::string& plaintext_mac() const    { return plaintext_mac_; }
    const std::string& ciphertext_mac() const   { return ciphertext_mac_; }
    const std::string& iv() const               { return iv_; }

    unsigned int position() const               { return position_; }

    void set_chunk_name(const std::string& name)     { chunk_name_ = name; } 
    void set_checksum(const std::string& sum)        { checksum_ = sum; }
    void set_plaintext_mac(const std::string& mac)   { plaintext_mac_ = mac; } 
    void set_ciphertext_mac(const std::string& mac)  { ciphertext_mac_ = mac; }
    void set_iv(const std::string& iv)               { iv_ = iv; }
    void set_position(const unsigned int position)   { position_ = position; }

    bool HasIv() const { return !iv_.empty(); }

private:
    std::string chunk_name_;        // Name of this particular chunk
    std::string checksum_;          // Hash of the chunk before encryption
    std::string plaintext_mac_;     // Hash of the chunk
    std::string ciphertext_mac_;    // Hash of the Iv
    std::string iv_;                // Iv used to encrypt chunk

    unsigned int position_;         // Position in the order of chunks
};

} // namespace
#endif

