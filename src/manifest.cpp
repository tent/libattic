
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
    m_pDb = NULL;
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
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{

    std::cout << " callback hit ... " << std::endl;
    int i;

    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");

    return 0;

}

void Manifest::Initialize()
{
    OpenSqliteDb();

}

void Manifest::Shutdown()
{
    CloseSqliteDb();
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
        if(m_pDb)
            std::cout<<"not nil"<<std::endl;
        // success
        std::cout << "Open database successfully" << std::endl;

        // Check if table exists
        const char* pexc = "CREATE TABLE IF NOT EXISTS infotable (filename TEXT, filepath TEXT, chunkname TEXT, chunkcount INT, filesize INT, postid TEXT, postversion INT, key BLOB, iv BLOB, PRIMARY KEY(filename ASC));";

        char* szError;
        rc = sqlite3_exec(m_pDb, pexc, callback, 0, &szError);
        std::cout << " rc : " << rc << std::endl;

        if(szError)
            std::cout<<"ERROR : " << szError << std::endl;

        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", szError);
            sqlite3_free(szError);
        }

        const char* p2 = " INSERT INTO infotable (filename) values ('Allen')";
        rc = sqlite3_exec(m_pDb, p2, callback, 0, &szError);

        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", szError);
            sqlite3_free(szError);
        }
        sqlite3_close(m_pDb);
    }
}

void Manifest::CloseSqliteDb()
{
    if(m_pDb)
    {
        sqlite3_close(m_pDb);
        m_pDb = 0;
    }

}

void Manifest::CreateTable()
{
    if(!m_pDb)
        return;

    const char* pexc = "CREATE TABLE IF NOT EXISTS infotable (filename TEXT, filepath TEXT, chunkname TEXT, chunkcount INT, filesize INT, postid TEXT, postversion INT, key BLOB, iv BLOB, PRIMARY KEY(filename ASC));";


}

void Manifest::PerformQuery(const char* pQuery)
{
    if(!m_pDb || !pQuery)
        return;

    char* szError;

    int rc = sqlite3_exec( m_pDb, 
                           pQuery, 
                           NULL, 
                           NULL,
                           &szError);

    if(rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", szError);
        sqlite3_free(szError);
    }

}

void Manifest::QueryForFile(const std::string &filename)
{

}

void Manifest::InsertFileInfo(const FileInfo* fi)
{

}

void Manifest::RemoveFile(const std::string &filename)
{

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

            // Name 
            std::string fn;
            (*itr).second->GetFileName(fn);

            line.append(fn.c_str());
            line.append("\t");

            // Path
            std::string fp;
            (*itr).second->GetFilePath(fp);

            line.append(fp.c_str());
            line.append("\t");

            // Chunk Name
            std::string chunkName;
            (*itr).second->GetChunkName(chunkName);

            line.append(chunkName.c_str());
            line.append("\t");

            // Chunk Count
            char szBuffer[256];
            memset(szBuffer, 0, sizeof(char)*256);
            snprintf(szBuffer, (sizeof(char)*256),  "%d", (*itr).second->GetChunkCount());

            line.append(szBuffer);
            line.append("\t");

            // Filesize
            memset(szBuffer, 0, sizeof(char)*256);
            snprintf(szBuffer, (sizeof(char)*256), "%d", (*itr).second->GetFileSize());

            line.append(szBuffer);
            line.append("\t");

            // Post ID
            std::string postid;
            (*itr).second->GetPostID(postid);

            line.append(postid.c_str());
            line.append("\t");

            // Version
            memset(szBuffer, 0, sizeof(char)*256);
            snprintf(szBuffer, (sizeof(char)*256),  "%d", (*itr).second->GetPostVersion());

            line.append(szBuffer);
            line.append("\t");

            // Key
            Credentials cred = (*itr).second->GetCredentials();

            char key[cred.GetKeySize()+1];
            memset(key, '\0', cred.GetKeySize()+1);
            memcpy(key, cred.key, cred.GetKeySize());

            line.append(key, cred.GetKeySize());
            line.append("\t");

            // IV
            char iv[cred.GetIvSize()+1];
            memset(iv, '\0', cred.GetIvSize()+1);
            memcpy(iv, cred.iv, cred.GetIvSize());

            line.append(iv, cred.GetIvSize());
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

    std::string fn;
    fi->GetFileName(fn);
    m_entries[fn] = fi;
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

