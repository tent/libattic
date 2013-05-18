#include "infotablehandler.h"

#include "logutils.h"

namespace attic {

InfoTableHandler::InfoTableHandler(sqlite3* db) 
                                   : TableHandler(db, "infotable") {
}

InfoTableHandler::~InfoTableHandler() { 
    TableHandler::~TableHandler(); 
}

bool InfoTableHandler::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (filename TEXT, filepath TEXT, chunkcount INT,";
    exc += " chunkdata BLOB, filesize INT, metapostid TEXT, credential_data TEXT,";
    exc += " postversion TEXT, encryptedkey BLOB, iv BLOB,";
    exc += " deleted INT, folder_post_id TEXT, alias_data TEXT,";
    exc += " PRIMARY KEY(filepath ASC, folder_post_id ASC, metapostid ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)
        log::LogString("info190234jk", error);
    return ret;
}

bool InfoTableHandler::InsertFileInfo(const FileInfo& fi) {
    return false;
}

bool InfoTableHandler::RemoveFileInfo(const std::string &filepath) {
    return false;
}

bool InfoTableHandler::IsFileInManifest(const std::string &filepath) {
    return false;
}


}//namespace
