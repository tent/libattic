#include "filepost.h"

#include "fileinfo.h"
#include "constants.h"
#include "errorcodes.h"
#include "crypto.h"

namespace attic { 

FilePost::FilePost(){
    set_type(cnst::g_attic_file_type);
}

FilePost::~FilePost() {}

void FilePost::InitializeFilePost(FileInfo* fi,  bool is_public) {
    if(fi) {
        // basic attic post info
        set_public(is_public);
        set_relative_path(fi->filepath());
        set_name(fi->filename());
        set_file_size(fi->file_size());
        // attic post key info
        set_key_data(fi->encrypted_key());
        set_iv_data(fi->file_credentials_iv());
        set_folder_post(fi->folder_post_id());
    }
}

void FilePost::Serialize(Json::Value& root) {
    Json::Value content(Json::objectValue);
    content["name"] = name_;
    content["path"] = relative_path_;
    
    std::string key_data;
    crypto::Base64EncodeString(key_data_, key_data);
    content["kdata"] = key_data;

    std::string iv_data;
    crypto::Base64EncodeString(iv_data_, iv_data);
    content["vdata"] = iv_data;

    content["size"] = file_size_;
    content["folder_post"] = folder_post_;

    set_content("file_content", content);

    Post::Serialize(root);
}

void FilePost::Deserialize(Json::Value& root) {
    std::cout<<" deserializing file post " << std::endl;
    Post::Deserialize(root);

    Json::Value content;
    get_content("file_content", content);

    name_           = content.get("name", "").asString();
    relative_path_  = content.get("path", "").asString();
    folder_post_    = content.get("folder_post", "").asString();

    std::string size = content.get("size", "").asString();
    file_size_ = atoi(size.c_str());

    std::string key_data = content["kdata"].asString();
    std::string iv_data = content["vdata"].asString();

    crypto::Base64DecodeString(key_data, key_data_);
    crypto::Base64DecodeString(iv_data, iv_data_);
}

} //namespace
