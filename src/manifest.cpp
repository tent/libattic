
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

bool Manifest::LoadManifest(std::string &szFilePath)
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

bool Manifest::WriteOutManifestHeader(std::ofstream &ofs)
{
    if(ofs)
    {
        char countBuf[256];
        snprintf(countBuf, (sizeof(char)*256), "%lu", m_entries.size());

        std::string manifestData;
        manifestData.append(countBuf);
        manifestData.append("\n");

        m_ofStream.write(manifestData.c_str(), manifestData.size());

        return true;
    }

    return false;
}

bool Manifest::WriteOutManifest()
{
    // TODO :: use a filepath
    m_ofStream.open(m_filePath.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    if(m_ofStream.is_open())
    {
        // Write out manifest specific data first.
        WriteOutManifestHeader(m_ofStream);
        
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
            snprintf(szBuffer, (sizeof(char)*256),  "%d", (*itr).second->GetChunkCount());

            line.append(szBuffer);
            line.append("\t");

            memset(szBuffer, 0, sizeof(char)*256);
            snprintf(szBuffer, (sizeof(char)*256), "%d", (*itr).second->GetFileSize());

            line.append(szBuffer);
            line.append("\t");

            line.append("\n");

            m_ofStream.write(line.c_str(), line.size());
        } 

        m_ofStream.close();
        return true;
    }

   return false; 
}


bool Manifest::InsertFileInfo(FileInfo* fi)
{
    if(!fi)
        return false;
    
    //TODO :: expand checks for valid file etc
    //        for now just insert and be done.

    m_entries[fi->GetFileName()] = fi;
    m_entryCount++;

    return true;
}

bool Manifest::CreateEmptyManifest()
{

    m_ofStream.open(m_filePath.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    WriteOutManifestHeader(m_ofStream);
    m_ofStream.close();
    return true;
}

