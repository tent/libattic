
#include "chunker.h"

#include <string.h> // Note* yes, this is different than <string>, be aware
#include <stdio.h>

Chunker::Chunker(const char* szChunkName, unsigned int nChunkSize)
{
    m_chunkName.append(szChunkName);
    m_chunkSize = nChunkSize;
}

Chunker::~Chunker()
{

}

unsigned int Chunker::ChunkFile(std::string &szFilePath)
{
    // TODO ::
    // - check if there are currently chunks with a given filename already in the 
    //   directory.
    // - create obfuscated chunk names
    // - find some way to extract the filename from the filepath
    //

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

bool Chunker::DeChunkFile(std::string &szManifestFilePath)
{

    return false;
}

bool Chunker::WriteChunk(char* szBuffer, std::string &szName)
{
    if(!szBuffer)
    {
        return false;
    }

    m_ofStream.open(szName.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

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

