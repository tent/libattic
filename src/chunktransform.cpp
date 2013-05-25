#include "chunktransform.h"

#include "crypto.h"
#include "compression.h"
#include "chunkinfo.h"

namespace attic {

ChunkTransform::ChunkTransform(const std::string& chunk, const std::string& file_key) {
    data_ = chunk;
    file_key_ = file_key;
}

ChunkTransform::~ChunkTransform() {

}

bool ChunkTransform::TransformOut() {
    GenerateChunkName();
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
    return true;
}

bool ChunkTransform::TransformIn(const ChunkInfo* ci) {
    if(ci) {
        chunk_iv_ = ci->iv();
        // Decode
        std::string decoded_data;
        Decode(data_, decoded_data);
        // Decrypt
        std::string decrypted_data;
        Decrypt(decoded_data, decrypted_data);
        // Decompress
        Decompress(decrypted_data, finalized_data_);
        return true;
    }
    return false;
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

void ChunkTransform::Decompress(const std::string& in, std::string& out) {
    compress::DecompressString(in, out);
    // generate plaintext hash
    crypto::GenerateHash(out, plaintext_hash_);
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

void ChunkTransform::Decrypt(const std::string& in, std::string& out) {
    crypto::GenerateHash(in, ciphertext_hash_);
    // generate ciphertext hash
    Credentials chunk_cred;
    chunk_cred.set_iv(chunk_iv_);
    chunk_cred.set_key(file_key_);
    crypto::DecryptStringGCM(in, chunk_cred, out);
}

void ChunkTransform::Encode(const std::string& in, std::string& out) {
    crypto::Base64EncodeString(in, out);
}

void ChunkTransform::Decode(const std::string& in, std::string& out) {
    crypto::Base64DecodeString(in, out);
}

void ChunkTransform::GenerateVerificationHash(std::string& out) {
    std::string ver;
    crypto::GenerateHexEncodedHmac(finalized_data_, ver);
    out = ver.substr(0, 64);
}

} // namespace
