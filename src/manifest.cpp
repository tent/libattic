
#include "manifest.h"

#include <string.h>
#include <iostream>

#include "fileinfo.h"

Manifest::Manifest()
{
    // Set the default filename
    m_fileName.append("manifest._mn");
    m_entryCount = 0;
}

Manifest::~Manifest()
{

}
    

bool Manifest::LoadManifestFile(std::string &szFilePath)
{

    m_ifStream.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(m_ifStream.is_open())
    {
        unsigned int line_count = 0;
        std::string line;
        // Read Line by line
        while(!m_ifStream.eof())
        {
            std::getline(m_ifStream, line);
            if(line_count == 0)
            {
                // header info
                std::cout<<line<<std::endl;
                continue;
            }
        }

        m_ifStream.close();
        return true;
    }

    return false;
}

bool Manifest::WriteOutManifest()
{
    // TODO :: use a filepath
    m_ofStream.open(m_fileName.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    if(m_ofStream.is_open())
    {
        // Write out manifest specific data first.
        char countBuf[256];
        snprintf(countBuf, sizeof(countBuf), "%d", m_entryCount);

        std::string manifestData;
        manifestData.append(countBuf);
        manifestData.append("\n");

        m_ofStream.write(manifestData.c_str(), manifestData.size());

        // Write out each entry
        
        std::string line;


        std::map<std::string, FileInfo*>::iterator itr; 
        for(itr = m_entries.begin(); itr != m_entries.end(); itr++)
        {
            line.clear();

            line.append((*itr).second->GetFileName().c_str());
            line.append("\t");

            line.append((*itr).second->GetFilePath().c_str());
            line.append("\t");

            line.append((*itr).second->GetChunkName().c_str());
            line.append("\t");

            char szBuffer[256];
            memset(szBuffer, 0, sizeof(char)*256);
            snprintf(countBuf, sizeof(countBuf), "%d", (*itr).second->GetChunkCount());

            line.append(szBuffer);
            line.append("\t");

            memset(szBuffer, 0, sizeof(char)*256);
            snprintf(countBuf, sizeof(countBuf), "%d", (*itr).second->GetFileSize());

            line.append(szBuffer);
            line.append("\t");

            line.append("\n");

            m_ofStream.write(manifestData.c_str(), manifestData.size());
        } 

        m_ofStream.close();
        return true;
    }

   return false; 
}
