#ifndef FILEPOST_H_
#define FILEPOST_H_
#pragma once

#include <string>
#include <vector>

#include "post.h"
#include "fileinfo.h"
#include "crypto.h"

namespace attic { 

// Cargo, is a file's sensative metadata. This needs to be serialized and encrypted, before
// it is sent off into the nether
struct Cargo : public JsonSerializable {
    std::string filename;
    std::string filepath;
    std::string plaintext_hash;

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);
};

/* FilePost
 *  Specific post file for a file's metadata. Just basic file info goes here. This post
 *  is used as an anchor for the file itself. Chunks have their own post groupings that 
 *  mention the filepost, this allows a user to query for all chunk posts mentioning a
 *  specific file post and reassembling the file that way.
 */

class FilePost : public Post {
    void SerializeChunkData(Json::Value& root);
    void DeserializeChunkData(Json::Value& root);
public:
    FilePost();
    FilePost(FileInfo& fi);
    ~FilePost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    // Note* methods to retrieve name, filepath, and hash data where purposefully left out
    //  this is senstative information stored in the file posts' cargo.
    const std::string& key_data() const             { return fi_.encrypted_key(); }
    const std::string& iv_data() const              { return fi_.file_credentials_iv(); }
    const std::string& folder_post() const          { return fi_.folder_post_id(); }
    unsigned int file_size() const                  { return fi_.file_size(); }
    const FileInfo::ChunkMap chunk_data()           { return *(fi_.GetChunkInfoList()); }
    const std::string& cargo() const                { return cargo_; }

    void set_file_info(const FileInfo& fi)          { fi_ = fi; }
    void set_cargo(const std::string& c)            { cargo_ = c; }

    const FileInfo& fi() const { return fi_; }
private:
    std::string cargo_; // NOTE* this should be encrypted
    FileInfo    fi_;
};

}//namespace
#endif

