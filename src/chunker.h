
#ifndef CHUNKER_H_
#define CHUNKER_H_
#pragma once

#include <string>
#include <fstream>

class FileInfo;

class Chunker
{
    bool VerifyAllChunkExistence(const std::string &szChunkName, std::string &szChunkDir, unsigned int uCount);
    bool WriteChunk(char* szBuffer, std::string &szName);
    void SetName(std::string &nameOut, int nCount);
public:
    Chunker(const char* szChunkName = "chunk", const char* szChunkDir = "./output", unsigned int nChunkSize = 400000);
    ~Chunker();

    unsigned int ChunkFile(FileInfo *fi);
    bool DeChunkFile(FileInfo *fi);

    void SetChunkDirectory(std::string &szDir) { m_chunkDir = szDir; }
    void SetChunkName(std::string &szName) { m_chunkName = szName; }

    std::string GetChunkDirectory() { return m_chunkDir; }
    std::string GetChunkName() { return m_chunkName; }

private:
    std::ifstream   m_ifStream;
    std::ofstream   m_ofStream;

    std::string     m_chunkName;
    std::string     m_chunkDir;

    unsigned int    m_chunkSize;
};

#endif

