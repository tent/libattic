#ifndef CHUNKPOST_H_
#define CHUNKPOST_H_
#pragma once

#include <string>
#include <vector>
#include <map>

#include "post.h"
#include "chunkinfo.h"
#include "fileinfo.h"

namespace attic { 

class ChunkPost : public Post {
public:
    typedef std::map<unsigned int, ChunkInfo> ChunkInfoList;
    ChunkPost();
    ~ChunkPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    unsigned int chunk_info_list_size() { return chunk_info_list_.size(); }
    ChunkInfoList* chunk_info_list() { return &chunk_info_list_; }

    int set_chunk_info_list(FileInfo::ChunkMap& list);

private:
    ChunkInfoList chunk_info_list_;

};

}//namespace
#endif

