#include "chunktransform.h"

#include "crypto.h"
#include "compression.h"
#include "chunkinfo.h"
#include "logutils.h"

namespace attic {

ChunkTransform::ChunkTransform(const std::string& chunk, const std::string& file_key) {
    data_ = chunk;
    //file_key_ = file_key;
    file_key_.append(file_key.c_str(), file_key.size());
}

ChunkTransform::~ChunkTransform() {

}

void ChunkTransform::Reset() {
    chunk_iv_.clear();
    file_key_.clear();
    name_.clear();
    plaintext_hash_.clear();
    ciphertext_hash_.clear();
    verification_hash_.clear();
    data_.clear();
    finalized_data_.clear();
}

bool ChunkTransform::TransformOut() {
    GenerateChunkName();
    // Compress
    std::string compressed_data;
    Compress(data_, compressed_data);
    // Encrypt
    std::string encrypted_data;
    if(Encrypt(compressed_data, encrypted_data)) {
        // Encode
        Encode(encrypted_data, finalized_data_);
        // Generate verification hash
        GenerateVerificationHash(verification_hash_);
        return true;
    }
    return false;
}

bool ChunkTransform::TransformIn(const ChunkInfo* ci) {
    if(ci) {
        chunk_iv_.clear();
        chunk_iv_ = ci->iv();
        // Decode
        std::string decoded_data;
        Decode(data_, decoded_data);
        // Decrypt
        std::string decrypted_data;
        if(Decrypt(decoded_data, decrypted_data)) {
            // Decompress
            Decompress(decrypted_data, finalized_data_);
            return true;
        }
        std::ostringstream err;
        err <<"------------ Transform IN ------------" << std::endl;
        err << " Failed to transform chunk : " << ci->chunk_name() << std::endl;
        std::string fk, iv;
        crypto::Base64EncodeString(file_key_, fk);
        crypto::Base64EncodeString(chunk_iv_, iv);
        err << " file key size : " << file_key_.size() << std::endl;
        err << " file key : " << fk << std::endl;
        err << " chunk iv size : " << chunk_iv_.size() << std::endl;
        err << " chunk iv : " << iv << std::endl;
        err << " data size : " << data_.size() << std::endl;
        err << "-------------------------------------" << std::endl;
        log::LogString("0210410123", err.str());
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

bool ChunkTransform::Encrypt(const std::string& in, std::string& out) {
    Credentials chunk_cred;
    crypto::GenerateIv(chunk_iv_);
    chunk_cred.set_iv(chunk_iv_);
    chunk_cred.set_key(file_key_);
    bool ret = crypto::Encrypt(in, chunk_cred, out);
    // generate ciphertext hash
    crypto::GenerateHash(out, ciphertext_hash_);
    return ret;
}

bool ChunkTransform::Decrypt(const std::string& in, std::string& out) {
    crypto::GenerateHash(in, ciphertext_hash_);
    // generate ciphertext hash
    Credentials chunk_cred;
    chunk_cred.set_iv(chunk_iv_);
    chunk_cred.set_key(file_key_);
    return crypto::Decrypt(in, chunk_cred, out);
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
