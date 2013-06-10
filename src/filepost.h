#ifndef FILEPOST_H_
#define FILEPOST_H_
#pragma once

#include <string>
#include <vector>

#include "post.h"
#include "fileinfo.h"

namespace attic { 



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

    const std::string& name() const                 { return fi_.filename(); }
    const std::string& relative_path() const        { return fi_.filepath(); }
    const std::string& key_data() const             { return fi_.encrypted_key(); }
    const std::string& iv_data() const              { return fi_.file_credentials_iv(); }
    const std::string& folder_post() const          { return fi_.folder_post_id(); }
    unsigned int file_size() const                  { return fi_.file_size(); }

    void set_file_info(const FileInfo& fi)          { fi_ = fi; }
private:
    FileInfo    fi_;
};

}//namespace
#endif

