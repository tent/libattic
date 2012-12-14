// Manifest file structure
// Within the manifest file itself the structure of data to read in
// is as follows. Line by line
//  Manifest specific: entry count, 
//      entry : file name, file path, chunk name, chunk count, file size
//      entry : ...

#ifndef MANIFEST_H_
#define MANIFEST_H_
#pragma once

#include <fstream>
#include <string>
#include <map>

#include <sqlite3.h>

class FileInfo;

class SelectResult
{ 
public:
    SelectResult() 
    {
        nRow = 0;
        nCol = 0;
    }

    char** results;
    int nRow;
    int nCol;
};

class Manifest
{
    // SQLite specific ///////////////////////////////////
    void OpenSqliteDb();
    void CloseSqliteDb();

    bool CreateTables();
    bool CreateInfoTable();
    bool CreateMetaTable();

    bool PerformQuery(const char* pQuery);
    bool PerformSelect(const char* pSelect, SelectResult &out);

    // InfoTable
    bool InsertFileInfoToDb(const FileInfo* fi);
    bool InsertCredentialsToDb(const FileInfo* fi);

    bool QueryForFileExistence(const std::string& filename);

    bool RemoveFileFromDb(const std::string &filename);
    void CheckIfTableExists(const std::string &tableName);

    // MetaTable
    unsigned int QueryForVersion();
    void QueryForMetaPostID(std::string &out);

    bool InsertVersionNumber(unsigned int version);
    bool InsertPostID(const std::string &postID);
    //////////////////////////////////////////////////////

    bool WriteOutManifestHeader(std::ofstream &ofs);
public:
    typedef std::map<std::string, FileInfo*> EntriesMap;

    // TODO pull this back to private
    bool QueryForFile(const std::string &filename, FileInfo* out);

    Manifest();
    ~Manifest();

    void Initialize();
    void Shutdown();

    bool CreateEmptyManifest();
    bool WriteOutManifest();    

    bool InsertFileInfo(FileInfo* fi);
    bool InsertFilePostID(const std::string &filename, const std::string &id);

    bool RemoveFileInfo(const std::string &filename);

    bool IsFileInManifest(const std::string &filename);

    FileInfo* RetrieveFileInfo(const std::string &s);

    void SetFilePath(std::string &filePath) { m_Filepath = filePath; }
    
    unsigned int GetEntryCount() { return m_EntryCount; }
    void SetEntryCount(unsigned int count) { m_EntryCount = count; }
    
    EntriesMap* GetEntries() { return &m_Entries; }

    unsigned int GetVersionNumber() { return QueryForVersion(); }//return m_VersionNumber; }
    void GetPostID(std::string &out) { }


private:
    EntriesMap          m_Entries;  // Do not delete entries, just clear the map.
                                    // FileInfoFactory will take care of deletion.

    sqlite3*            m_pDb;
    std::ifstream       m_ifStream;
    std::ofstream       m_ofStream;

    // Manifest specific data
    std::string         m_Filepath;     // path to manifest file
    std::string         m_Filename;     // filename of the manifest
    unsigned int        m_EntryCount;   // Number of entries in the manifest
    
    unsigned int        m_VersionNumber; // Version Number of sqlitedb
};

#endif

