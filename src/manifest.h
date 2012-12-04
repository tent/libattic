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

class Manifest
{
    // SQLite specific ///////////////////////////////////
    void OpenSqliteDb();
    void CloseSqliteDb();
    void CreateTable();
    void PerformQuery(const char* pQuery);
    void QueryForFile(const std::string &filename);
    void InsertFileInfo(const FileInfo* fi);
    void RemoveFile(const std::string &filename);
    void CheckIfTableExists(const std::string &tableName);
    //////////////////////////////////////////////////////

    bool WriteOutManifestHeader(std::ofstream &ofs);
public:
    typedef std::map<std::string, FileInfo*> EntriesMap;

    Manifest();
    ~Manifest();

    void Initialize();
    void Shutdown();

    bool CreateEmptyManifest();
    bool WriteOutManifest();    

    bool InsertFileInfo(FileInfo* fi);
    bool RemoveFileInfo(const std::string &filename);

    bool IsFileInManifest(const std::string &filename);

    FileInfo* RetrieveFileInfo(const std::string &s);

    void SetFilePath(std::string &filePath) { m_filePath = filePath; }
    
    unsigned int GetEntryCount() { return m_entryCount; }
    void SetEntryCount(unsigned int count) { m_entryCount = count; }
    
    EntriesMap* GetEntries() { return &m_entries; }

private:
    EntriesMap          m_entries;  // Do not delete entries, just clear the map.
                                    // FileInfoFactory will take care of deletion.

    sqlite3*            m_pDb;
    std::ifstream       m_ifStream;
    std::ofstream       m_ofStream;

    // Manifest specific data
    std::string         m_filePath;     // path to manifest file
    std::string         m_fileName;     // filename of the manifest
    unsigned int        m_entryCount;   // Number of entries in the manifest
};

#endif

