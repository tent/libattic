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
#include <deque>
#include <string>
#include <map>
#include <sqlite3.h>

#include "fileinfo.h"
#include "folder.h"

namespace attic { 

class SelectResult { 
    friend class Manifest;
public:
    SelectResult() {
        row_ = 0;
        col_ = 0;
    }
    ~SelectResult() {
        sqlite3_free_table(results_);
    }

    std::string operator[](const unsigned int n) { return results_[n]; }

    int row() const { return row_; }
    int col() const { return col_; }
private:
    char** results_;
    int row_;
    int col_;
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

    void ExtractFileInfoResults(const SelectResult& res, const int step, FileInfo& out);
    bool PushBackAlias(const std::string& filepath, const std::string& alias);

    void ExtractFolderInfoResults(const SelectResult& res, const int step, Folder& out);
public:
    typedef std::map<std::string, FileInfo*> EntriesMap;
    typedef std::deque<FileInfo> FileInfoList;

    Manifest();
    ~Manifest();

    int Initialize();
    int Shutdown();

    bool CreateEmptyManifest();

    // File Info
    bool InsertFileInfo(const FileInfo& fi);
    bool UpdateFilePostID(const std::string &filename, const std::string &id);
    bool UpdateFileVersion(const std::string& filepath, const std::string& version);
    bool UpdateFileDeleted(const std::string& filepath, const int val);
    bool UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath);
    bool UpdateFilename(const std::string& filepath, const std::string& new_filename);
    bool UpdatePastAlias(const std::string& filepath, const std::string& alias_data);
    bool UpdateFileFolderPostId(const std::string& filepath, const std::string& post_id);


    bool QueryForFile(const std::string &filepath, FileInfo& out);
    bool QueryForFileByPostId(const std::string& post_id, FileInfo& out);
    int QueryAllFiles(FileInfoList& out);

    int QueryAllFilesForFolder(const std::string& folderid, FileInfoList& out);

    bool RemoveFileInfo(const std::string &filepath);
    bool IsFileInManifest(const std::string &filename);

    // Folder Table
    bool InsertFolderInfo(const std::string& folderpath, 
                          const std::string& folderpostid,
                          const std::string& parentpostid);
    bool QueryForFolder(const std::string& folderpath, Folder& out);
    bool QueryForFolderByPostId(const std::string& post_id, Folder& out);

    bool IsFolderInManifest(const std::string& folderpath);
    bool IsFolderInManifestWithID(const std::string& folderid);

    bool UpdateFolderPostId(const std::string& folderpath, const std::string& folderpostid);
    bool UpdateFolderParentPostId(const std::string& folderpath, const std::string& parent_post_id);
    bool UpdateFolderPath(const std::string& folderid, const std::string& folderpath);

    bool GetFolderPostID(const std::string& folderpath, std::string& out);
    bool GetFolderPath(const std::string& folder_manifest_id, std::string& path_out);
    bool GetFolderID(const std::string& folderpath, std::string& out);
    bool RemoveFolderData(const std::string& folderpath);

    bool UpdateAllFileInfoForFolder(const std::string& folderid);

    void SetDirectory(std::string &filepath); 
private:
    sqlite3*            db_;

    // Manifest specific data
    std::string         filepath_;     // path to manifest file
};

} //namespace
#endif

