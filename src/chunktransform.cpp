#include "chunktransform.h"

#include "crypto.h"
#include "compression.h"

namespace attic {

ChunkTransform::ChunkTransform(const std::string& chunk, const std::string& file_key) {
    data_ = chunk;
    file_key_ = file_key;
    GenerateChunkName();
}

ChunkTransform::~ChunkTransform() {

}

void ChunkTransform::Transform() {
    // Compress
    std::string compressed_data;
    Compress(data_, compressed_data);
    // Encrypt
    std::string encrypted_data;
    Encrypt(compressed_data, encrypted_data);
    // Encode
    Encode(encrypted_data, finalized_data_);
    // Generate verification hash
    GenerateVerificationHash(verification_hash_);
}

void ChunkTransform::GenerateChunkName() {
    if(!data_.empty()) {
        crypto::GenerateHash(data_, plaintext_hash_);
        crypto::HexEncodeString(plaintext_hash_, name_);
    }
}

void ChunkTransform::Compress(const std::string& in, std::string& out) {
    compress::CompressString(in, out);
}

void ChunkTransform::Encrypt(const std::string& in, std::string& out) {
    Credentials chunk_cred;
    crypto::GenerateIv(chunk_iv_);
    chunk_cred.set_iv(chunk_iv_);
    chunk_cred.set_key(file_key_);
    crypto::EncryptStringGCM(in, chunk_cred, out);
    // generate ciphertext hash
    crypto::GenerateHash(out, ciphertext_hash_);
}

void ChunkTransform::Encode(const std::string& in, std::string& out) {
    crypto::Base64EncodeString(in, out);
}

void ChunkTransform::GenerateVerificationHash(std::string& out) {
    std::string ver;
    crypto::GenerateHexEncodedHmac(finalized_data_, ver);
    out = ver.substr(0, 64);
}

} // namespace
