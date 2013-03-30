#ifndef FILEPOST_H_
#define FILEPOST_H_
#pragma once

#include <string>
#include <vector>

#include "post.h"

namespace attic { 

class FileInfo;

class FilePost : public Post {
public:
   FilePost();
    ~FilePost();

    void Serialize(Json::Value& root);  
    void SerializeChunkPosts(Json::Value& val);
    void SerializeChunkIds(Json::Value& val);

    void Deserialize(Json::Value& root);
    void DeserializeChunkPosts(Json::Value& val);
    void DeserializeChunkIds(Json::Value& val);

    void PushBackChunkPostId(const std::string& postId) { chunk_posts_.push_back(postId); }
    void PushBackChunkIdentifier(const std::string& id) { chunk_ids_.push_back(id); }

    std::vector<std::string> GetChunkPosts() const { return chunk_posts_; }

    const std::string& name() const          { return name_; }
    const std::string& relative_path() const { return relative_path_; }
    const std::string& key_data() const      { return key_data_; }
    const std::string& iv_data() const       { return iv_data_; }
    const std::string& checksum() const      { return checksum_; }
    unsigned int file_size() const           { return file_size_; }
    int deleted() const                      { return deleted_; }

    void set_name(const std::string& name)          { name_ = name; }
    void set_relative_path(const std::string& path) { relative_path_ = path; }
    void set_key_data(const std::string& key)       { key_data_ = key; }
    void set_iv_data(const std::string& iv)         { iv_data_ = iv; }
    void set_checksum(const std::string& checksum)  { checksum_ = checksum; }
    void set_file_size(unsigned int size)           { file_size_ = size; }
    void set_deleted(int del)                       { deleted_ = del; }
private:
    std::vector<std::string> chunk_posts_;
    std::vector<std::string> chunk_ids_;

    // Attic specific post                       
    std::string name_;                  // Name of file
    std::string relative_path_;         // Relative file path within attic folder 
    std::string key_data_;
    std::string iv_data_;
    std::string checksum_;

    unsigned int file_size_;
    int deleted_;
};

}//namespace
#endif

