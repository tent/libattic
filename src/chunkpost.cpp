
#include "chunkpost.h"

ChunkPost::ChunkPost()
{
    m_pChunkList = NULL;

}

ChunkPost::~ChunkPost()
{
    if(m_pChunkList)
    {
        // DO NOT delete anything, this is taken care of elsewhere
        m_pChunkList->clear();
        m_pChunkList = NULL;
    }

}

void ChunkPost::Serialize(Json::Value& root)
{

}

void ChunkPost::Deserialize(Json::Value& root)
{

}


