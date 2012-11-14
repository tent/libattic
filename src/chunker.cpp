
#include "chunker.h"

#include <string.h> // Note* yes, this is different than <string>, be aware
#include <stdio.h>

#include "fileinfo.h"
#include "utils.h"

Chunker::Chunker(const char* szChunkName, const char* szChunkDir, unsigned int nChunkSize)
{
    m_chunkName.append(szChunkName);
    m_chunkDir.append(szChunkDir);
    m_chunkSize = nChunkSize;
}

Chunker::~Chunker()
{

}

unsigned int Chunker::ChunkFile(FileInfo *fi)
{
    // TODO ::
    // - check if there are currently chunks with a given filename already in the 
    //   directory.
    // - !!create obfuscated chunk names!! perform a hash on the filepath and use that.
    // - find some way to extract the filename from the filepath
    
    if(!fi)
        return 0;

    // Get FilePath
    std::string szFilePath = fi->GetFilePath();
    // Resolve Chunk Name TODO :: HASH 
    fi->SetChunkName(m_chunkName);

    unsigned int count = 0;
    m_ifStream.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if (m_ifStream.is_open())
    {
        std::string name;
        char* szBuffer = new char[m_chunkSize];

        // Build the Chunk
        while(!m_ifStream.eof())
        {
            // Zero out memory
            memset(szBuffer, 0, sizeof(char)*m_chunkSize);

            SetName(name, count);

            if(!WriteChunk(szBuffer, name))
            {
                m_ifStream.close();
                if(szBuffer)
                {   
                    delete szBuffer;
                    szBuffer = 0;
                }

                return 0;
            }
            
            count++;
        }

        m_ifStream.close();
        if(szBuffer)
        {   
            delete szBuffer;
            szBuffer = 0;
        }
    }

    return count;
}

bool Chunker::DeChunkFile(FileInfo *fi)
{
    if(!fi)
        return false;
    // Create output path
    std::string outputPath;
    outputPath = m_chunkDir + fi->GetFileName();

    if(!VerifyAllChunkExistence(fi->GetChunkName(), m_chunkDir, fi->GetChunkCount()))
        return false;

    // Cycle through chunks
    //
    // Dump into output stream
    //

    // Open stream for output file
    if(m_ofStream.is_open())
        m_ofStream.close();

    m_ofStream.open(outputPath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!m_ofStream.is_open())
        return false;

    std::string chunkName;
    std::string inputPath;
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
        inputPath = m_chunkDir + "/" + chunkName; 

        // Open input stream

        // Find all chunks
        // 
        // Read the chunk
        // shove it into the output stream

    }

    return false;
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

bool Chunker::WriteChunk(char* szBuffer, std::string &szName)
{
    if(!szBuffer)
    {
        return false;
    }
    // resolve path
    std::string path;
    path = m_chunkDir + "/" + szName;

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

void Chunker::SetName(std::string &nameOut, int nCount)
{
    // Set the name
    nameOut.clear();
    nameOut.append(m_chunkName.c_str());
    nameOut.append("_");

    char countBuf[256];
    snprintf(countBuf, sizeof(countBuf), "%d", nCount);
    nameOut.append(countBuf); 
}

