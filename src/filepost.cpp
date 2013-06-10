#include "filepost.h"

#include "fileinfo.h"
#include "constants.h"
#include "errorcodes.h"
#include "crypto.h"

namespace attic { 

FilePost::FilePost() {
    set_type(cnst::g_attic_file_type);
    set_public(false);
}
FilePost::FilePost(FileInfo& fi) {
    set_type(cnst::g_attic_file_type);
    set_public(false);
    fi_ = fi;
}

FilePost::~FilePost() {}

void FilePost::Serialize(Json::Value& root) {
    Json::Value content(Json::objectValue);
    content["name"] = fi_.filename();
    content["path"] = fi_.filepath();;
    
    std::string key_data;
    crypto::Base64EncodeString(fi_.encrypted_key(), key_data);
    content["kdata"] = key_data;

    std::string iv_data;
    crypto::Base64EncodeString(fi_.file_credentials_iv(), iv_data);
    content["vdata"] = iv_data;

    content["size"] = fi_.file_size();
    content["folder_post"] = fi_.folder_post_id();

    Json::Value chunks;
    SerializeChunkData(chunks);
    content["chunks"] = chunks;

    // Serialize Chunk data
    set_content("file_content", content);
    Post::Serialize(root);
}

void FilePost::SerializeChunkData(Json::Value& root) {
    FileInfo::ChunkMap::iterator itr = fi_.GetChunkInfoList()->begin();
    for(;itr!= fi_.GetChunkInfoList()->end(); itr++) {
        Json::Value c;
        jsn::SerializeObject(&itr->second, c);
        root.append(c);
    }
}

void FilePost::Deserialize(Json::Value& root) {
    std::cout<<" deserializing file post " << std::endl;
    Post::Deserialize(root);

    Json::Value content;
    get_content("file_content", content);

    fi_.set_filename(content.get("name", "").asString());
    fi_.set_filepath(content.get("path", "").asString());
    fi_.set_folder_post_id(content.get("folder_post", "").asString());
    std::string size = content.get("size", "").asString();
    fi_.set_file_size(size.c_str());

    std::string key_data;
    crypto::Base64DecodeString(content["kdata"].asString(), key_data);
    fi_.set_encrypted_key(key_data);

    std::string iv_data;
    crypto::Base64DecodeString(content["vdata"].asString(), iv_data);
    fi_.set_file_credentials_iv(iv_data);

    // Deserialize Chunk Data
    Json::Value chunks = content["chunks"];
    DeserializeChunkData(chunks);
}

void FilePost::DeserializeChunkData(Json::Value& root) {
    Json::ValueIterator itr = root.begin();
    for(;itr != root.end(); itr++) {
        ChunkInfo c;
        jsn::DeserializeObject(&c, (*itr));
        fi_.PushChunkBack(c);
    }
}


} //namespace

