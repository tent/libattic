#ifndef MANIFEST_H_
#define MANIFEST_H_
#pragma once

#include <fstream>
#include <deque>
#include <string>
#include <map>
#include <sqlite3.h>

#include "tablehandler.h"
#include "fileinfo.h"
#include "folder.h"

namespace attic { 

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
    typedef std::deque<Folder> FolderList;

    Manifest();
    ~Manifest();

    int Initialize();
    int Shutdown();

    bool CreateEmptyManifest();

    // File Info
    bool IsFileInManifest(const std::string &filepath);
    bool InsertFileInfo(const FileInfo& fi);
    bool UpdateFilePostID(const std::string &filename, const std::string &id);
    bool UpdateFileVersion(const std::string& filepath, const std::string& version);
    bool UpdateFileDeleted(const std::string& filepath, const int val);
    bool UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath);
    bool UpdateFilename(const std::string& filepath, const std::string& new_filename);
    bool UpdatePastAlias(const std::string& filepath, const std::string& alias_data);
    bool UpdateFileFolderPostId(const std::string& filepath, const std::string& post_id);
    bool UpdateFileChunkCount(const std::string& filepath, const std::string& chunk_count);

    bool QueryForFile(const std::string &filepath, FileInfo& out);
    bool QueryForFileByPostId(const std::string& post_id, FileInfo& out);
    int QueryAllFiles(FileInfoList& out);
    int QueryAllFilesForFolder(const std::string& folderid, FileInfoList& out);
    int QueryAllFoldersForFolder(const std::string& folderid, FolderList& out);

    bool MarkAllFilesDeletedInFolder(const std::string& folderid);
    bool RemoveFileInfo(const std::string &filepath);

    // Folder Table
    bool InsertFolderInfo(const std::string& folderpath, 
                          const std::string& folderpostid,
                          const std::string& parentpostid,
                          const bool deleted);

    bool QueryForFolder(const std::string& folderpath, Folder& out);
    bool QueryForFolderByPostId(const std::string& post_id, Folder& out);

    bool IsFolderInManifest(const std::string& folderpath);
    bool IsFolderInManifestWithID(const std::string& post_id);

    bool UpdateFolderPostId(const std::string& folderpath, const std::string& folderpostid);
    bool UpdateFolderParentPostId(const std::string& folderpath, const std::string& parent_post_id);
    bool UpdateFolderPath(const std::string& post_id, const std::string& folderpath);
    bool UpdateFolderDeleted(const std::string& folderpath, bool del);

    bool GetFolderPostID(const std::string& folderpath, std::string& out);
    bool GetFolderPath(const std::string& folder_manifest_id, std::string& path_out);
    bool GetFolderID(const std::string& folderpath, std::string& out);
    bool GetAliasData(const std::string& folderpath, std::string& out);

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

