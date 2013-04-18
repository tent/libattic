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

#include "fileinfo.h"
#include "folder.h"

namespace attic { 

class SelectResult { 
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

class Manifest {
    // TODO :: abstract table specific methods
    // SQLite specific ///////////////////////////////////
    int OpenSqliteDb();
    int CloseSqliteDb();

    bool CreateTables();
    bool CreateInfoTable();
    bool CreateFolderTable();
    bool CreateFolderEntryTable();

    bool PerformQuery(const std::string& query) const;
    bool PerformSelect(const std::string& select, SelectResult &out) const;

    // InfoTable
    void CheckIfTableExists(const std::string &tableName);

public:
    typedef std::map<std::string, FileInfo*> EntriesMap;

    Manifest();
    ~Manifest();

    int Initialize();
    int Shutdown();

    bool CreateEmptyManifest();

    // File Info
    bool InsertFileInfo(const FileInfo& fi);
    bool UpdateFilePostID(const std::string &filename, const std::string &id);
    bool UpdateFileChunkPostID(const std::string &filename, const std::string &id);
    bool UpdateFileVersion(const std::string& filepath, const std::string& version);
    bool UpdateFileDeleted(const std::string& filepath, const int val);
    bool UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath);
    bool UpdateFilename(const std::string& filepath, const std::string& new_filename);

    bool QueryForFile(const std::string &filename, FileInfo& out);
    int QueryAllFiles(std::vector<FileInfo>& out);

    bool RemoveFileInfo(const std::string &filepath);
    bool IsFileInManifest(const std::string &filename);

    // Folder Table
    bool IsFolderInManifest(const std::string& folderpath);
    bool IsFolderInManifestWithID(const std::string& folderid);
    bool InsertFolderInfo(const std::string& folderpath, const std::string& folderpostid);
    bool UpdateFolderPostId(const std::string& folderpath, const std::string& folderpostid);

    bool GetFolderPostID(const std::string& folderpath, std::string& out);
    bool GetFolderID(const std::string& folderpath, std::string& out);

    bool RemoveFolderData(const std::string& folderpath);

    // Folder Entry
    bool IsFolderEntryInManifest(const std::string& filepath);
    bool InsertFolderEnrty(const std::string& folderid, 
                           const std::string& metapostid, 
                           const std::string& type,
                           const std::string& filepath);


    bool SetFolderEntryMetapostID(const std::string& filepath, const std::string& metapostid);
    bool GetFolderEntryMetapostID(const std::string& filepath, std::string& out);

    void SetDirectory(std::string &filepath); 
private:
    sqlite3*            db_;

    // Manifest specific data
    std::string         filepath_;     // path to manifest file
};

} //namespace
#endif

