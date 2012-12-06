
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
        if(!m_pDb)
        {
            std::cout << " invlid db instance " << std::endl;
            return;
        }

        CreateTable();
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

    PerformQuery(pexc);

}

void Manifest::PerformQuery(const char* pQuery)
{
    if(!m_pDb || !pQuery)
        return;

    char* szError = NULL;
    int rc = sqlite3_exec( m_pDb, 
                           pQuery, 
                           callback, 
                           NULL,
                           &szError);

    if(rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", szError);
        sqlite3_free(szError);
        szError = NULL;
    }

}

// int sqlite3_get_table(
//                       sqlite3 *db,          /* An open database */
//                       const char *zSql,     /* SQL to be evaluated */
//                       char ***pazResult,    /* Results of the query */
//                       int *pnRow,           /* Number of result rows written here */
//                       int *pnColumn,        /* Number of result columns written here */
//                       char **pzErrmsg       /* Error msg written here */
//                       );
//                      void sqlite3_free_table(char **result);
void Manifest::PerformSelect(const char* pSelect)
{
    if(!m_pDb || !pSelect)
        return;

    char *szError = NULL;
    char ***results = NULL;
    int *pnRow = NULL;
    int *pnCol = NULL;
    char **pzErr = NULL;

    std::cout << " here " << std::endl;

    int rc = sqlite3_get_table( m_pDb,
                                pSelect,     /* SQL to be evaluated */
                                results,     /* Results of the query */
                                pnRow,       /* Number of result rows written here */
                                pnCol,      /* Number of result columns written here */
                                pzErr        /* Error msg written here */
                               );

    std::cout << " here " << std::endl;
    //sqlite3_free_table(*results);

}

void Manifest::QueryForFile(const std::string &filename)
{
    const char* pexc = "SELECT * FROM infotable";
    std::cout<< " EXECING : " << pexc << std::endl;
    //PerformQuery(pexc);
    PerformSelect(pexc);
}

void Manifest::InsertFileInfoToDb(const FileInfo* fi)
{
    if(!fi)
        return;

    std::string filename, filepath, chunkname, postid, key, iv;

    fi->GetFileName(filename);
    fi->GetFilePath(filepath);
    fi->GetChunkName(chunkname);
    fi->GetPostID(postid);
    fi->GetKey(key);
    fi->GetIv(iv);


    char* pexc = new char[1024];
    snprintf( pexc,
              1024, 
              "INSERT INTO infotable (filename, filepath, chunkname, chunkcount, filesize, postid, postversion, key, iv) VALUES (%s, %s, %s, %u, %u, %s, %d, %s, %s);",
              filename.c_str(),
              filepath.c_str(),
              chunkname.c_str(),
              fi->GetChunkCount(),
              fi->GetFileSize(),
              postid.c_str(),
              fi->GetPostVersion(),
              key.c_str(),
              iv.c_str()); 

    std::cout << " KEY : " << key << std::endl;
    std::cout << " IV : " << iv << std::endl;

    std::cout << " INSERT : \n" << pexc << std::endl;
    PerformQuery(pexc);

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

    InsertFileInfoToDb(fi);

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

