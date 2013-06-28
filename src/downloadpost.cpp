#include "downloadpost.h"

#include "constants.h"

namespace attic { 

DownloadPost::DownloadPost() {
    set_type(cnst::g_attic_download_type);
}

DownloadPost::~DownloadPost() {

}

void DownloadPost::Serialize(Json::Value& root) {
    Json::Value content(Json::objectValue);
    content["filename"] = filename_;
    content["filesize"] = filesize_;
    content["plaintext_hash"] = plaintext_hash_;


    set_content("file_content", content);
    Post::Serialize(root);
}

void DownloadPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);
    Json::Value content;
    get_content("file_content", content);
    filename_           = content.get("filename", "").asString();
    filesize_           = content.get("filesize", "").asString();
    plaintext_hash_     = content.get("plaintext_hash", "").asString();
}

}//namespace

