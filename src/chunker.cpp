
#include "chunker.h"

#include <iostream>
#include <string.h> // Note* yes, this is different than <string>, be aware
#include <stdio.h>

#include "fileinfo.h"
#include "utils.h"
#include "chunkinfo.h"

Chunker::Chunker(unsigned int nChunkSize)
{
    m_chunkSize = nChunkSize;
}

Chunker::~Chunker()
{

}

ret::eCode Chunker::ChunkFile(FileInfo *fi, const std::string &filepath, const std::string &chunkDir)
{
    // TODO ::
    // - check if there are currently chunks with a given filename already in the 
    //   directory.
    // - !!create obfuscated chunk names!! perform a hash on the filepath and use that.
    // - find some way to extract the filename from the filepath
    
    if(!fi)
        return ret::A_FAIL_INVALID_PTR;

    // Resolve Chunk Name   
    std::string chunkName;
    fi->GetChunkName(chunkName);

    if(chunkName.empty())
    {
        utils::GenerateRandomString(chunkName);
        fi->SetChunkName(chunkName);
    }

    unsigned int count = 0;
    //std::cout<< "Filepath " << filepath <<std::endl;
    m_ifStream.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if (!m_ifStream.is_open())
    {
        return ret::A_FAIL_OPEN_FILE;
    }

    std::string name;
    char* szBuffer = new char[m_chunkSize];

    // Build the Chunk
    while(!m_ifStream.eof())
    {

        // Zero out memory
        memset(szBuffer, 0, sizeof(char)*m_chunkSize);

        SetName(chunkName, name, count);

        std::string hashOut;
        if(!WriteOutChunk(szBuffer, chunkDir, name, hashOut))
        {
            m_ifStream.close();
            if(szBuffer)
            {   
                delete szBuffer;
                szBuffer = 0;
            }

            return ret::A_FAIL_WRITE_CHUNK;
        }

        // Create Chunk info
        ChunkInfo* ci = new ChunkInfo(name, hashOut);
        // Push back into fileinfo
        fi->PushChunkBack(*ci);

        delete ci;
        ci = NULL;
        
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
#include <cstring>
ret::eCode Chunker::DeChunkFile( FileInfo *fi, 
                                 const std::string &outboundPath, 
                                 const std::string &chunkDir)
{
    if(!fi)
        return ret::A_FAIL_INVALID_PTR;
    // Create output path

    std::string chunkName;
    fi->GetChunkName(chunkName);

    if(!VerifyAllChunkExistence(chunkName, chunkDir, fi->GetChunkCount()))
        return ret::A_FAIL_VERIFY_CHUNKS;

    // Open stream for output file
    if(m_ofStream.is_open())
    {
        m_ofStream.close();
    }

    //std::cout<<" attempting to open outbound path : " << outboundPath << std::endl;
    m_ofStream.open(outboundPath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!m_ofStream.is_open())
    {
        // strerror, <- STANDARD DEF, NON STANDARD IMPLEMENTATION, MAY NOT WORK
        //std::cout << " ERROR : " << std::strerror(errno) << std::endl;
        return ret::A_FAIL_OPEN_FILE;
    }

    std::string fileName;
    std::string inputPath;
    // Cycle through chunks
    for(unsigned int i=0; i<fi->GetChunkCount(); i++)
    {
        // assemble chunk name
        char szBuffer[256];
        memset(szBuffer, 0, (sizeof(char)*256));
        snprintf(szBuffer, (sizeof(char)*256), "%d", i);

        fileName = chunkName + "_" + szBuffer;
        // assemble path
        inputPath.clear();
        inputPath = chunkDir + "/" + fileName;

        // Open input stream
        m_ifStream.open(inputPath.c_str(), std::ifstream::in | std::ifstream::binary);

        if(!m_ifStream.is_open())
        {
            return ret::A_FAIL_OPEN_FILE;
        }

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

bool Chunker::VerifyAllChunkExistence( const std::string &chunkName, 
                                       const std::string &chunkDir, 
                                       unsigned int uCount)
{
    //std::cout<< "CHUNK COUNT : " << uCount << std::endl;
    std::string path;
    for(unsigned int i=0; i<uCount; i++)
    {
        path.clear();

        char szBuffer[256];
        memset(szBuffer, 0, (sizeof(char)*256));
        snprintf(szBuffer, (sizeof(char)*256), "%d", i);

        path = chunkDir + "/" + chunkName + "_" + szBuffer;

        //std::cout<<" PATH : " << path << std::endl;
        if(!utils::CheckFilesize(path))
            return false;
    }

    return true;
}

bool Chunker::WriteOutChunk( char* szBuffer, 
                             const std::string &chunkDir, 
                             const std::string &name,
                             std::string& hashOut)
{
    if(!szBuffer)
    {
        return false;
    }
    // resolve path
    std::string path;
    path = chunkDir + "/" + name;

    m_ofStream.open(path.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    if(m_ofStream.is_open())
    {
        m_ifStream.read(szBuffer, m_chunkSize);

        // Calculate hash
        m_Crypto.GenerateHash(szBuffer, hashOut);

        int readcount = m_ifStream.gcount();

        m_ofStream.write(szBuffer, readcount);
        m_ofStream.close();
        return true;
    }

    return false;
}

void Chunker::SetName(const std::string &basename, std::string &nameOut, int nCount)
{
    // Set the name
    nameOut.clear();
    nameOut.append(basename.c_str());
    nameOut.append("_");

    char countBuf[256];
    snprintf(countBuf, sizeof(countBuf), "%d", nCount);
    nameOut.append(countBuf); 
}

