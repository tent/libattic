#ifndef INFOTABLEHANDLER_H_
#define INFOTABLEHANDLER_H_
#pragma once

#include <deque>
#include "tablehandler.h"
#include "fileinfo.h"

namespace attic {

class InfoTableHandler : public TableHandler {
public:
    typedef std::deque<FileInfo> FileInfoList;

    InfoTableHandler(sqlite3* db);
    ~InfoTableHandler();

    bool CreateTable();
    // Insert
    bool InsertFileInfo(const FileInfo& fi, std::string& error_out);
    bool RemoveFileInfo(const std::string &filepath);
    bool IsFileInManifest(const std::string &filepath);
    // Modify
    bool UpdateFilePostID(const std::string &filename, const std::string &id);
    bool UpdateFileVersion(const std::string& filepath, const std::string& version);
    bool UpdateFileDeleted(const std::string& filepath, const int val);
    bool UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath);
    bool UpdateFilename(const std::string& filepath, const std::string& new_filename);
    bool UpdatePastAlias(const std::string& filepath, const std::string& alias_data);
    bool UpdateFileFolderPostId(const std::string& filepath, const std::string& post_id);
    // Query
    bool QueryForFile(const std::string &filepath, FileInfo& out);
    bool QueryForFileByPostId(const std::string& post_id, FileInfo& out);
    int QueryAllFiles(FileInfoList& out);
    int QueryAllFilesForFolder(const std::string& folderid, FileInfoList& out);
    // Delete
    bool MarkAllFilesDeletedInFolder(const std::string& folderid);
private:
};

} // namespace

#endif

