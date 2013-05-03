#include "chunktransform.h"

#include "crypto.h"
#include "compression.h"

ChunkTransform::ChunkTransform(const std::string& chunk, const std::string& file_key) {
    data_ = chunk;
    file_key_ = file_key;
    GenerateChunkName();
}

ChunkTransform::~ChunkTransform() {

}

void ChunkTransform::Transform() {

}

void ChunkTransform::GenerateChunkName() {
    if(!data.empty()) {
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
}

void ChunkTransform::Encode(const std::string& in, std::string& out) {

}

