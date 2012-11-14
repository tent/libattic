
#ifndef CHUNKER_H_
#define CHUNKER_H_
#pragma once

#include <string>
#include <fstream>

class Chunker
{
    bool WriteChunk(char* szBuffer, std::string &szName);
    void SetName(std::string &nameOut, int nCount);
public:
    Chunker(const char* szChunkName = "chunk", unsigned int nChunkSize = 400000);
    ~Chunker();

    unsigned int ChunkFile(std::string &szFilePath);
    bool DeChunkFile(std::string &szManifestFilePath);

private:
    std::ifstream   m_ifStream;
    std::ofstream   m_ofStream;
    std::string     m_chunkName;

    unsigned int    m_chunkSize;
};

#endif

