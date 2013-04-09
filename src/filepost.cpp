#include "filepost.h"

#include <stdio.h>

#include "fileinfo.h"
#include "constants.h"
#include "errorcodes.h"
#include "crypto.h"

namespace attic { 

FilePost::FilePost(){
    set_type(cnst::g_attic_file_type);
    deleted_ = 0;
}

FilePost::~FilePost() {}

void FilePost::InitializeFilePost(FileInfo* fi,  bool is_public) {
    if(fi) {
        // basic attic post info
        set_public(is_public);
        set_relative_path(fi->filepath());
        set_name(fi->filename());
        set_file_size(fi->file_size());
        set_deleted(fi->deleted());
        // attic post key info
        set_key_data(fi->encrypted_key());
        set_iv_data(fi->file_credentials_iv());
        // set chunk info
        PushBackChunkPostId(fi->chunk_post_id()); // TODO :: change this to a mention
        FileInfo::ChunkMap* chunk_list = fi->GetChunkInfoList();
        if(chunk_list) {
            std::cout<<" CHUNK LIST SIZE : " << chunk_list->size() << std::endl;
            FileInfo::ChunkMap::iterator itr = chunk_list->begin();

            std::string identifier;
            for(;itr != chunk_list->end(); itr++) {
                identifier.clear();
                identifier = itr->second.checksum();
                PushBackChunkIdentifier(identifier);
            }
        }
        else {
            std::cout<<" INVALID CHUNK LIST : " << std::endl;
        }
    }
}

void FilePost::Serialize(Json::Value& root) {
    Json::Value content(Json::objectValue);
    content["name"] = name_;
    content["path"] = relative_path_;

    char del[256] = {'\0'};
    snprintf(del, 256, "%d", deleted_);
    content["deleted"] = del;
    
    Json::Value chunkposts;//(Json::objectValue);
    SerializeChunkPosts(chunkposts);
    content["chunk_posts"] = chunkposts;

    Json::Value chunkids;
    SerializeChunkIds(chunkids);
    content["chunk_ids"] = chunkids;

    std::string key_data;
    crypto::Base64EncodeString(key_data_, key_data);
    content["kdata"] = key_data;

    std::string iv_data;
    crypto::Base64EncodeString(iv_data_, iv_data);
    content["vdata"] = iv_data;

    content["size"] = file_size_;

    set_content("file_content", content);

    Post::Serialize(root);
}

void FilePost::SerializeChunkPosts(Json::Value& val) {
    jsn::SerializeVector(chunk_posts_, val);
}

void FilePost::SerializeChunkIds(Json::Value& val) { 
    jsn::SerializeVector(chunk_ids_, val);
}

void FilePost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    Json::Value content;
    get_content("file_content", content);

    name_           = content.get("name", "").asString();
    relative_path_  = content.get("path", "").asString();
    std::string del = content.get("deleted", "").asString();
    deleted_ = atoi(del.c_str());

    std::string size = content.get("size", "").asString();
    file_size_ = atoi(size.c_str());

    DeserializeChunkPosts(content["chunk_posts"]);
    DeserializeChunkIds(content["chunk_ids"]);

    std::string key_data = content["kdata"].asString();
    std::string iv_data = content["vdata"].asString();

    crypto::Base64DecodeString(key_data, key_data_);
    crypto::Base64DecodeString(iv_data, iv_data_);
}

void FilePost::DeserializeChunkPosts(Json::Value& val) {
    jsn::DeserializeIntoVector(val, chunk_posts_);
}

void FilePost::DeserializeChunkIds(Json::Value& val) {
    jsn::DeserializeIntoVector(val, chunk_ids_);
}

} //namespace
