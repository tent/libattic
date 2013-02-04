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
#include <vector>
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
    // TODO :: abstract table specific methods
    // SQLite specific ///////////////////////////////////
    void OpenSqliteDb();
    void CloseSqliteDb();

    bool CreateTables();
    bool CreateInfoTable();
    bool CreateMetaTable();
    bool CreateFolderTable();


    bool PerformQuery(const char* pQuery) const;
    bool PerformSelect(const char* pSelect, SelectResult &out) const;

    // InfoTable
    bool InsertFileDataToInfoTable(const FileInfo* fi);
    bool InsertCredentialsToDb(const FileInfo* fi);

    bool QueryForFileExistence(const std::string& filename);

    bool RemoveFileFromDb(const std::string &filename);
    void CheckIfTableExists(const std::string &tableName);

    // MetaTable
    unsigned int QueryForVersion() const;
    void QueryForMetaPostID(std::string &out) const;

    bool InsertVersionNumber(unsigned int version) const;
    bool InsertPostID(const std::string &postID) const;

        void SetIsDirty(bool dirty) { m_Dirty = dirty; }  // Deprecated
public:
    typedef std::map<std::string, FileInfo*> EntriesMap;

    Manifest();
    ~Manifest();

    void Initialize();
    void Shutdown();

    bool CreateEmptyManifest();

    // File Info
    bool InsertFileInfo(const FileInfo* fi);
    bool InsertFilePostID(const std::string &filename, const std::string &id);
    bool InsertFileChunkPostID(const std::string &filename, const std::string &id);

    bool QueryForFile(const std::string &filename, FileInfo* out);
    int QueryAllFiles(std::vector<FileInfo>& out);

    bool RemoveFileInfo(const std::string &filename);
    bool IsFileInManifest(const std::string &filename);

    
    // Folder Table
    int InsertFolderDataToFolderTable(const FileInfo* fi);
    bool RemoveFolderData(const std::string& folderpath);


    bool QueryForFolderData( const std::string& folderpath,
                             std::string &nameOut,
                             std::string &pathOut,
                             std::string &childrenOut,
                             std::string &postidOut);

    unsigned int GetEntryCount()              { return m_EntryCount; }
    unsigned int GetVersionNumber() const     { return QueryForVersion(); }//return m_VersionNumber; }
    //void GetPostID(std::string &out) const    { QueryForMetaPostID(out); }
    bool GetIsDirty()                         { return m_Dirty; } 

    void SetPostID(const std::string &id)   { InsertPostID(id); }
    void SetEntryCount(unsigned int count)  { m_EntryCount = count; }
    void SetFilePath(std::string &filePath) { m_Filepath = filePath; }

    void CompareAndMergeDb(sqlite3* pDb);

private:
    sqlite3*            m_pDb;
    std::ifstream       m_ifStream;
    std::ofstream       m_ofStream;

    // Manifest specific data
    std::string         m_Filepath;     // path to manifest file
    std::string         m_Filename;     // filename of the manifest
    unsigned int        m_EntryCount;   // Number of entries in the manifest
    
    unsigned int        m_VersionNumber; // Version Number of sqlitedb

    bool                m_Dirty; // Has manifest been written to since last sync
};

#endif

