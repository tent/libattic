#ifndef SHAREDPOSTTABLE_H_
#define SHAREDPOSTTABLE_H_
#pragma once

#include <vector>
#include <string>
#include "constants.h"
#include "tablehandler.h"

namespace attic { 

class SharedPostTable : public TableHandler { 
    friend class Manifest;

    SharedPostTable(sqlite3* db) : TableHandler(db, cnst::g_sharedposttable) {}
    SharedPostTable(const SharedPostTable& rhs) : TableHandler(rhs.db(), rhs.table_name()) {}
    SharedPostTable operator=(const SharedPostTable& rhs) { return *this; }
public:

    ~SharedPostTable() {}
    
    bool CreateTable();

    bool InsertSharedFileInfo(const std::string& shared_post_id, // post id of the shared post
            const std::string& file_post_id,                     // post id of the orignal file post
            const std::string& entity);                          // entity url shared with

    bool QueryForSharedPostsByFilePost(const std::string& file_post_id,
            std::vector<std::string>& post_ids);
    bool QueryForSharedPostsByEntity(const std::string& entity,
            std::vector<std::string>& post_ids);
    bool QueryForFilePost(const std::string& shared_post_id, 
            std::string& post_id_out);
};

} // namespace

#endif

