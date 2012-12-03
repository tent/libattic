
#include "manifest.h"

#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <vector>

#include "fileinfo.h"
#include "utils.h"



Manifest::Manifest()
{
    // Set the default filename
    m_fileName.append("manifest._mn");
    m_entryCount = 0;
}

Manifest::~Manifest()
{

}
/*
 * Order to write out (and read in)
 * Manifest Header
 * - number of entries (unsigned long) 
 *
 * Entry
 * - Filename (str)
 * - Filepath (str)
 * - ChunkName (str)
 * - ChunkCount (unsigned int)
 * - FileSize (unsigned int)
 * - PostID
 * - PostVersion
 */

 void Manifest::Initialize()
 {

 }
 
 void Manifest::Shutdown()
 {

 }

void Manifest::OpenSqliteDb()
{
    int rc = sqlite3_open("test.db", &m_pDb);
    if( rc )
    {
        // failed
        std::cout<< "Can't open database: " << sqlite3_errmsg(m_pDb);
    }
    else
    {
        // success
        std::cout << "Open database successfully" << std::endl;
    }
}

void Manifest::CheckIfTableExists(const std::string &tableName)
{
    if(!m_pDb)
        return;



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

            std::string chunkName;
            (*itr).second->GetChunkName(chunkName);

            line.append(chunkName.c_str());
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

            line.append((*itr).second->GetPostID().c_str());
            line.append("\t");

            memset(szBuffer, 0, sizeof(char)*256);
            snprintf(szBuffer, (sizeof(char)*256),  "%d", (*itr).second->GetPostVersion());

            line.append(szBuffer);
            line.append("\n"); // End the line

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

bool Manifest::RemoveFileInfo(const std::string &filename)
{
    EntriesMap::iterator itr;
    itr = m_entries.find(filename);

    if(itr == m_entries.end())
        return false;

    m_entries.erase(itr);

    return true;
}


FileInfo* Manifest::RetrieveFileInfo(const std::string &s)
{
    EntriesMap::iterator itr;
    itr = m_entries.find(s);

    if(itr == m_entries.end())
        return 0;
    return itr->second;
}

bool Manifest::CreateEmptyManifest()
{

    m_ofStream.open(m_filePath.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    WriteOutManifestHeader(m_ofStream);
    m_ofStream.close();
    return true;
}

bool Manifest::IsFileInManifest(const std::string &filename)
{
    if(m_entries.find(filename) != m_entries.end())
        return true;

    return false;
}

