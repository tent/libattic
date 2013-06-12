#ifndef FILETABLE_H_
#define FILETABLE_H_
#pragma once

#include <string>
#include "constants.h"
#include "tablehandler.h"
#include "fileinfo.h"

namespace attic {

/* FileTable deals with all file meta data operations
 *
 * */
class FileTable : public TableHandler {
    friend class Manifest;

    void ExtractFileInfoResults(const SelectResult& res, const int step, FileInfo& out);

    FileTable(sqlite3* db) : TableHandler(db, cnst::g_filetable) {}
    FileTable(const FileTable& rhs) : TableHandler(rhs.db(), rhs.table_name()) {}
    FileTable operator=(const FileTable& rhs) { return *this; }
public:
    typedef std::deque<FileInfo> FileInfoList;
    
    ~FileTable() {}

    bool CreateTable();
    bool IsFileInManifest(const std::string &filepath);

    bool InsertFileInfo(const FileInfo& fi);
    bool RemoveFileInfo(const std::string &filepath);

    bool set_file_post_id(const std::string &filepath, const std::string &id);
    bool set_file_version(const std::string& filepath, const std::string& version);
    bool set_file_deleted(const std::string& filepath, const int val);
    bool set_filepath(const std::string& old_filepath, const std::string& new_filepath);
    bool set_filename(const std::string& filepath, const std::string& new_filename);
    bool set_folder_post_id(const std::string& filepath, const std::string& post_id);
    bool set_chunk_count(const std::string& filepath, const std::string& chunk_count);
    bool set_plaintext_hash(const std::string& filepath, const std::string& hash);

    bool QueryForFile(const std::string &filepath, FileInfo& out);
    bool QueryForFileByPostId(const std::string& post_id, FileInfo& out);
    bool QueryAllFiles(FileInfoList& out);
    bool QueryAllFilesForFolder(const std::string& folderid, FileInfoList& out);

    bool MarkAllFilesDeletedInFolder(const std::string& folderid);
};

} //namespace
#endif

