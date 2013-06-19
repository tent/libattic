#ifndef FOLDERTABLE_H_
#define FOLDERTABLE_H_
#pragma once

#include <string>
#include "constants.h"
#include "tablehandler.h"
#include "folder.h"

namespace attic {

class FolderTable : public TableHandler {
    friend class Manifest;

    void ExtractFolderInfoResults(const SelectResult& res, const int step, Folder& out);

    FolderTable(sqlite3* db) : TableHandler(db, cnst::g_foldertable) {}
    FolderTable(const FolderTable& rhs) : TableHandler(rhs.db(), rhs.table_name()) {}
    FolderTable operator=(const FolderTable& rhs) { return *this; }
public:
    typedef std::deque<Folder> FolderList;

    ~FolderTable() {}

    bool CreateTable();

    bool InsertFolderInfo(const std::string& foldername, 
                          const std::string& folderpostid,
                          const std::string& parentpostid,
                          const bool deleted);

    bool RemoveFolderData(const std::string& foldername);

    bool IsFolderInManifest(const std::string& post_id);
    bool IsFolderDeleted(const std::string& post_id);

    bool QueryForFolder(const std::string& foldername, 
                        const std::string parent_post_id, 
                        Folder& out);

    bool QueryForFolder(const std::string& foldername, Folder& out); // Depricated
    bool QueryForFolderByPostId(const std::string& post_id, Folder& out);
    bool QueryAllFoldersForFolder(const std::string& folderid, FolderList& out);

    bool set_folder_post_id(const std::string& post_id, const std::string& new_post_id);
    bool set_folder_parent_post_id(const std::string& post_id, const std::string& parent_post_id);
    bool set_foldername(const std::string& post_id, const std::string& foldername);
    bool set_folder_deleted(const std::string& post_id, bool del);

    bool GetFolderPostID(const std::string& foldername, std::string& out);
    bool GetFolderPath(const std::string& folder_post_id, std::string& path_out);
    bool GetFolderId(const std::string& foldername, std::string& out);
};

} // namespace
#endif

