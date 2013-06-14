#include "chunkinfo.h"

#include "crypto.h"

namespace attic {

ChunkInfo::ChunkInfo() {}

ChunkInfo::ChunkInfo(const std::string& chunkname, const std::string& checksum) {
    position_ = 0;
    chunk_name_ = chunkname;
    checksum_ = checksum;
    group_ = -1;
}

ChunkInfo::~ChunkInfo() {}

void ChunkInfo::Serialize(Json::Value& root) {
    std::string chunkname, plaintextmac, ciphertextmac, iv, position,  digest;
    crypto::Base64EncodeString(chunk_name_, chunkname);
    crypto::Base64EncodeString(plaintext_mac_, plaintextmac);
    crypto::Base64EncodeString(ciphertext_mac_, ciphertextmac);
    crypto::Base64EncodeString(iv_, iv);
    crypto::Base64EncodeString(digest_, digest);
    char pos[256] = {'\0'};
    snprintf(pos, 256, "%u", position_);
    crypto::Base64EncodeString(std::string(pos), position);

    root["chunk_name"] = chunkname;
    root["plaintext_mac"] = plaintextmac;
    root["ciphertext_mac"] = ciphertextmac; 
    root["iv"] = iv;
    root["position"] = position;
    root["digest"] = digest;
}

void ChunkInfo::Deserialize(Json::Value& root) {
    std::string chunkname = root.get("chunk_name", "").asString();
    std::string plaintextmac = root.get("plaintext_mac", "").asString();
    std::string ciphertextmac = root.get("ciphertext_mac", "").asString();
    std::string iv = root.get("iv", "").asString();
    std::string position = root.get("position", "").asString();
    std::string digest = root.get("digest", "").asString();

    crypto::Base64DecodeString(chunkname, chunk_name_);
    crypto::Base64DecodeString(plaintextmac, plaintext_mac_);
    crypto::Base64DecodeString(ciphertextmac, ciphertext_mac_);
    crypto::Base64DecodeString(iv, iv_);
    std::string decodepos;
    crypto::Base64DecodeString(position, decodepos);
    position_ = atoi(decodepos.c_str());
    crypto::Base64DecodeString(digest, digest_);
}

} //namespace
