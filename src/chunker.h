
#ifndef CHUNKER_H_
#define CHUNKER_H_
#pragma once

#include <string>
#include <fstream>

#include "errorcodes.h"

class FileInfo;

class Chunker
{
    bool VerifyAllChunkExistence( const std::string &chunkName, 
                                  const std::string &chunkDir, 
                                  unsigned int uCount);

    bool WriteChunk( char* szBuffer, 
                     const std::string &chunkDir, 
                     const std::string &name);

    void SetName( const std::string &basename, 
                  std::string &nameOut, 
                  int nCount);

public:
    Chunker(unsigned int nChunkSize = 400000);
    ~Chunker();

    ret::eCode ChunkFile( FileInfo *fi, 
                          const std::string &filepath, 
                          const std::string &chunkDir);

    ret::eCode DeChunkFile( FileInfo *fi, 
                            const std::string &outboundPath, 
                            const std::string &chunkDir);

    unsigned int GetChunkSize() { return m_chunkSize; }

    void SetChunkSize(unsigned int uChunkSize) { m_chunkSize = uChunkSize; }

private:
    std::ifstream   m_ifStream;
    std::ofstream   m_ofStream;

    unsigned int    m_chunkSize;
};

#endif

