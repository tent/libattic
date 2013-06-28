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

    const std::string& filename() const             { return filename_; }
    const std::string& filesize() const             { return filesize_; } 
    const std::string& plaintext_hash() const       { return plaintext_hash_; }

    void set_filename(const std::string& filename)      { filename_ = filename; }
    void set_filesize(const std::string& filesize)      { filesize_ = filesize; }
    void set_plaintext_hash(const std::string& hash)    { plaintext_hash_ = hash; }
private:
    std::string filename_;
    std::string filesize_;
    std::string plaintext_hash_;
};


} // namespace

#endif

