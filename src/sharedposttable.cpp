#include "sharedposttable.h"

#include "logutils.h"

namespace attic { 

bool SharedPostTable::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (shared_post_id TEXT, file_post_id TEXT, entity TEXT,";
    exc += " PRIMARY KEY(shared_post_id ASC, file_post_id ASC, entity ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_102894", error);
    return ret; 
}

bool SharedPostTable::InsertSharedFileInfo(const std::string& shared_post_id,
            const std::string& file_post_id,
            const std::string& entity) {
    std::string query;
    query += "INSERT OR REPLACE INTO ";
    query += table_name();
    query += " (shared_post_id, file_post_id, entity)";
    query += " VALUES (?,?,?);";

    std::string error;
    bool ret = false;
    ret = PrepareStatement(query, error);        if(!ret) {log::ls("m_890",error);return ret;}
    ret = BindText(1, shared_post_id, error);    if(!ret) {log::ls("m_891",error);return ret;}
    ret = BindText(2, file_post_id, error);      if(!ret) {log::ls("m_892",error);return ret;}
    ret = BindText(3, entity, error);            if(!ret) {log::ls("m_893",error);return ret;}
    ret = StepStatement(error);                  if(!ret) {log::ls("m_894",error);return ret;}
    ret = FinalizeStatement(error);              if(!ret) {log::ls("m_895",error);return ret;}
    return ret;
}

bool SharedPostTable::QueryForSharedPostsByFilePost(const std::string& file_post_id,
        std::vector<std::string>& post_ids) {
    bool ret = false;
    std::string query;
    query += "SELECT shared_post_id FROM ";
    query += table_name();
    query += " WHERE file_post_id=\"";
    query += file_post_id;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                ExtractFileInfoResults(res, step, out);
                ret = true;
            }
        }
    }
    else {
        log::LogString("manifest_8i09255", error);
    }

    return ret;
}

bool SharedPostTable::QueryForSharedPostsByEntity(const std::string& entity,
        std::vector<std::string>& post_ids) {
    bool ret = false;
    return ret;
}

bool SharedPostTable::QueryForFilePost(const std::string& shared_post_id, 
        std::string& post_id_out) {
    bool ret = false;
    return ret;
}

}//namespace


