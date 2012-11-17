
#include "chunker.h"

#include <string.h> // Note* yes, this is different than <string>, be aware
#include <stdio.h>

#include "fileinfo.h"
#include "utils.h"

Chunker::Chunker(unsigned int nChunkSize)
{
    m_chunkSize = nChunkSize;
}

Chunker::~Chunker()
{

}

ret::eCode Chunker::ChunkFile(FileInfo *fi, std::string &szFilePath, std::string &szChunkDir)
{
    // TODO ::
    // - check if there are currently chunks with a given filename already in the 
    //   directory.
    // - !!create obfuscated chunk names!! perform a hash on the filepath and use that.
    // - find some way to extract the filename from the filepath
    
    if(!fi)
        return ret::A_FAIL_INVALID_PTR;

    // Resolve Chunk Name   
    // TODO :: Generate Chunk names 
    std::string chunkName;
    chunkName.append("chunk");

    fi->SetChunkName(chunkName);

    unsigned int count = 0;
    std::cout<< "Filepath " << szFilePath <<std::endl;
    m_ifStream.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if (!m_ifStream.is_open())
    {
        return ret::A_FAIL_OPEN;
    }

    std::string name;
    char* szBuffer = new char[m_chunkSize];

    // Build the Chunk
    while(!m_ifStream.eof())
    {
        // Zero out memory
        memset(szBuffer, 0, sizeof(char)*m_chunkSize);

        SetName(chunkName, name, count);

        if(!WriteChunk(szBuffer, szChunkDir, name))
        {
            m_ifStream.close();
            if(szBuffer)
            {   
                delete szBuffer;
                szBuffer = 0;
            }

            return ret::A_FAIL_WRITE_CHUNK;
        }
        
        count++;
    }

    m_ifStream.close();
    if(szBuffer)
    {   
        delete szBuffer;
        szBuffer = 0;
    }

    fi->SetChunkCount(count);

    return ret::A_OK;
}

ret::eCode Chunker::DeChunkFile(FileInfo *fi, std::string &szOutboundPath, std::string &szChunkDir)
{
    if(!fi)
        return ret::A_FAIL_INVALID_PTR;
    // Create output path
    std::string outputPath;
    outputPath = szChunkDir + "/" + fi->GetFileName();

    if(!VerifyAllChunkExistence(fi->GetChunkName(), szChunkDir, fi->GetChunkCount()))
        return ret::A_FAIL_VERIFY_CHUNKS;

    // Open stream for output file
    if(m_ofStream.is_open())
        m_ofStream.close();

    m_ofStream.open(outputPath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!m_ofStream.is_open())
        return ret::A_FAIL_OPEN;

    std::string chunkName;
    std::string inputPath;
    // Cycle through chunks
    for(unsigned int i=0; i<fi->GetChunkCount(); i++)
    {
        chunkName.clear();
        // assemble chunk name
        char szBuffer[256];
        memset(szBuffer, 0, (sizeof(char)*256));
        snprintf(szBuffer, (sizeof(char)*256), "%d", i);

        chunkName = fi->GetChunkName() + "_" + szBuffer;
        // assemble path
        inputPath.clear();
        inputPath = szChunkDir + "/" + chunkName; 

        // Open input stream
        m_ifStream.open(inputPath.c_str(), std::ifstream::in | std::ifstream::binary);

        if(!m_ifStream.is_open())
            return ret::A_FAIL_OPEN;

        // Get Size of Chunk
        m_ifStream.seekg(0, std::ifstream::end);
        unsigned int size = m_ifStream.tellg();
        m_ifStream.seekg(0, std::ifstream::beg);
        
        // Create a temp buffer
        char* pBuf = new char[size];

        // Read the chunk
        m_ifStream.read(pBuf, size);

        // shove it into the output stream
        int readcount = m_ifStream.gcount();
        m_ofStream.write(pBuf, readcount);
        m_ifStream.close();
        delete pBuf;
        pBuf = 0;
 
    }
    m_ofStream.close();

    return ret::A_OK;
}

bool Chunker::VerifyAllChunkExistence(const std::string &szChunkName, std::string &szChunkDir, unsigned int uCount)
{
    std::string path;
    for(unsigned int i=0; i<uCount; i++)
    {
        path.clear();

        char szBuffer[256];
        memset(szBuffer, 0, (sizeof(char)*256));
        snprintf(szBuffer, (sizeof(char)*256), "%d", i);

        path = szChunkDir + "/" + szChunkName + "_" + szBuffer;
        if(!utils::CheckFileSize(path))
            return false;
    }

    return true;
}

bool Chunker::WriteChunk(char* szBuffer, std::string &szChunkDir, std::string &szName)
{
    if(!szBuffer)
    {
        return false;
    }
    // resolve path
    std::string path;
    path = szChunkDir + "/" + szName;

    m_ofStream.open(path.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    if(m_ofStream.is_open())
    {
        m_ifStream.read(szBuffer, m_chunkSize);

        int readcount = m_ifStream.gcount();

        m_ofStream.write(szBuffer, readcount);
        m_ofStream.close();
        return true;
    }

    return false;
}

void Chunker::SetName(std::string &szBaseName, std::string &szNameOut, int nCount)
{
    // Set the name
    szNameOut.clear();
    szNameOut.append(szBaseName.c_str());
    szNameOut.append("_");

    char countBuf[256];
    snprintf(countBuf, sizeof(countBuf), "%d", nCount);
    szNameOut.append(countBuf); 
}

