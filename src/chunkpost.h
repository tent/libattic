
#ifndef CHUNKPOST_H_
#define CHUNKPOST_H_
#pragma once

#include <string>
#include <vector>

#include "post.h"

class ChunkInfo;

class ChunkPost : public Post
{
public:
    ChunkPost();
    ~ChunkPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    void SetChunkInfoList(std::vector<ChunkInfo*>* pList) { m_pChunkList = pList; }

private:
    std::vector<ChunkInfo*>*    m_pChunkList;

};


#endif

