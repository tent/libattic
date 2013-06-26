#ifndef DOWNLOADPOST_H_
#define DOWNLOADPOST_H_
#pragma once

#include "post.h"

namespace attic { 

class DownloadPost : public Post {
public:
    DownloadPost();
    ~DownloadPost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    const std::string& filename() const         { return filename_; }
    const std::string& file_size() const        { return file_size_; } 
    const std::string& plaintext_hash() const    { return plaintext_hash_; }

    void set_filename(const std::string& filename)      { filename_ = filename; }
    void set_file_size(const std::string& file_size)    { file_size_ = file_size; }
    void set_plaintext_mac(const std::string& hash)     { plaintext_hash_ = hash; }
private:
    std::string filename_;
    std::string file_size_;
    std::string plaintext_hash_;
};


} // namespace

#endif

