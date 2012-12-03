
#ifndef CHUNKER_H_
#define CHUNKER_H_
#pragma once

#include <string>
#include <fstream>

#include "errorcodes.h"

class FileInfo;

class Chunker
{
    bool VerifyAllChunkExistence( const std::string &szChunkName, 
                                  const std::string &szChunkDir, 
                                  unsigned int uCount);

    bool WriteChunk( char* szBuffer, 
                     const std::string &szChunkDir, 
                     const std::string &szName);

    void SetName( const std::string &szBaseName, 
                  std::string &szNameOut, 
                  int nCount);

public:
    Chunker(unsigned int nChunkSize = 400000);
    ~Chunker();

    ret::eCode ChunkFile( FileInfo *fi, 
                          const std::string &szFilePath, 
                          const std::string &szChunkDir);

    ret::eCode DeChunkFile( FileInfo *fi, 
                            const std::string &szOutboundPath, 
                            const std::string &szChunkDir);

    unsigned int GetChunkSize() { return m_chunkSize; }

    void SetChunkSize(unsigned int uChunkSize) { m_chunkSize = uChunkSize; }

private:
    std::ifstream   m_ifStream;
    std::ofstream   m_ofStream;

    unsigned int    m_chunkSize;
};

#endif

