#include "sharedfilepost.h"

#include "crypto.h"
#include "constants.h"

namespace attic { 

SharedFilePost::SharedFilePost() {
    set_type(cnst::g_attic_shared_file_type);
}

SharedFilePost::~SharedFilePost() {}

void SharedFilePost::Serialize(Json::Value& root) {
    Json::Value shared_cred(Json::objectValue);

    std::string key_data;
    crypto::Base64EncodeString(encrypted_key_, key_data);
    shared_cred["key"] = key_data;

    std::string iv_data;
    crypto::Base64EncodeString(iv_, iv_data);
    shared_cred["iv"] = iv_data;

    std::string nonce_data;
    crypto::Base64EncodeString(nonce_, nonce_data);
    shared_cred["nonce"] = nonce_data;

    shared_cred["filename"] = filename_;
    shared_cred["owner"] = owner_;

    Json::Value chunks;
    SerializeChunkData(chunks);
    shared_cred["chunks"] = chunks;

    set_content("shared_credentials", shared_cred);
    Post::Serialize(root);
}

void SharedFilePost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    Json::Value shared_cred(Json::objectValue);
    get_content("shared_credentials", shared_cred);

    std::string key_data;
    crypto::Base64DecodeString(shared_cred["key"].asString(), key_data);
    encrypted_key_ = key_data;

    std::string iv_data;
    crypto::Base64DecodeString(shared_cred["iv"].asString(), iv_data);
    iv_ = iv_data;

    std::string nonce_data;
    crypto::Base64DecodeString(shared_cred["nonce"].asString(), nonce_data);
    nonce_ = nonce_data;

    filename_ = shared_cred.get("filename", "").asString();
    owner_ = shared_cred.get("owner", "").asString();

    // Deserialize Chunk Data
    Json::Value chunks = shared_cred["chunks"];
    DeserializeChunkData(chunks);
}

void SharedFilePost::SerializeChunkData(Json::Value& root) {
    FileInfo::ChunkMap::iterator itr = chunk_data_.begin();
    for(;itr!= chunk_data_.end(); itr++) {
        Json::Value c;
        jsn::SerializeObject(&itr->second, c);
        root.append(c);
    }
}

void SharedFilePost::DeserializeChunkData(Json::Value& root) {
    Json::ValueIterator itr = root.begin();
    for(;itr != root.end(); itr++) {
        ChunkInfo c;
        jsn::DeserializeObject(&c, (*itr));
        chunk_data_[c.chunk_name()] = c;
    }
}

} //namespace

