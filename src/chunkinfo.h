
#ifndef CHUNKINFO_H_
#define CHUNKINFO_H_
#pragma once

#include <string>

class ChunkInfo
{
public:
    ChunkInfo() {}
    ~ChunkInfo() {}

    void GetChunkName(std::string& out) const   { out = m_ChunkName; }
    void GetChecksum(std::string& out) const    { out = m_Checksum; }

    void SetChunkName(const std::string& name) { m_ChunkName = name; } 
    void SetChecksum(const std::string& sum) { m_Checksum = sum; }

private:
    std::string m_ChunkName;
    std::string m_Checksum;

};

#endif

