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
    int OpenSqliteDb();
    void CloseSqliteDb();

    bool CreateTables();
    bool CreateInfoTable();
    bool CreateFolderTable();

    bool PerformQuery(const std::string& query) const;
    bool PerformSelect(const std::string& select, SelectResult &out) const;

    // InfoTable
    bool QueryForFileExistence(const std::string& filename);
    void CheckIfTableExists(const std::string &tableName);

public:
    typedef std::map<std::string, FileInfo*> EntriesMap;

    Manifest();
    ~Manifest();

    int Initialize();
    void Shutdown();

    bool CreateEmptyManifest();

    // File Info
    bool InsertFileInfo(const FileInfo* fi);
    bool InsertFilePostID(const std::string &filename, const std::string &id);
    bool InsertFileChunkPostID(const std::string &filename, const std::string &id);

    bool QueryForFile(const std::string &filename, FileInfo* out);
    int QueryAllFiles(std::vector<FileInfo>& out);

    bool RemoveFileInfo(const std::string &filepath);
    bool IsFileInManifest(const std::string &filename);

    // Folder Table
    int InsertFolderDataToFolderTable(const FileInfo* fi);
    bool RemoveFolderData(const std::string& folderpath);


    bool QueryForFolderData( const std::string& folderpath,
                             std::string &nameOut,
                             std::string &pathOut,
                             std::string &childrenOut,
                             std::string &postidOut);

    void SetDirectory(std::string &filepath); 
private:
    sqlite3*            m_pDb;
    std::ifstream       m_ifStream;
    std::ofstream       m_ofStream;

    // Manifest specific data
    std::string         m_Filepath;     // path to manifest file
};

#endif

