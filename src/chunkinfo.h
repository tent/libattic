
#ifndef CHUNKINFO_H_
#define CHUNKINFO_H_
#pragma once

#include <iostream>
#include <string>

class ChunkInfo
{
public:
    ChunkInfo() {}
    ChunkInfo(const std::string& chunkName, const std::string& hash)
    {
        m_ChunkName = chunkName;
        m_CheckSum = hash;

        std::cout<<"CHUNK NAME : " << m_ChunkName << std::endl;
        std::cout<<"CHUNK HASH : " << m_CheckSum << std::endl;
    }

    ~ChunkInfo() {}

    void GetChunkName(std::string& out) const   { out = m_ChunkName; }
    void GetChecksum(std::string& out) const    { out = m_CheckSum; }

    void SetChunkName(const std::string& name) { m_ChunkName = name; } 
    void SetChecksum(const std::string& sum) { m_CheckSum = sum; }

private:
    std::string m_ChunkName;
    std::string m_CheckSum;
};

#endif

