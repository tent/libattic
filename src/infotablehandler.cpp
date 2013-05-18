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

bool InfoTableHandler::InsertFileInfo(const FileInfo& fi, std::string& error_out) {
    // Prepare data
    std::string chunkdata;
    fi.GetSerializedChunkData(chunkdata);

    std::string alias_data;
    fi.GetSerializedAliasData(alias_data);

    std::string alias_encoded;
    crypto::Base64EncodeString(alias_data, alias_encoded);

    std::string cred_data = fi.file_credentials().asString();
    std::string b64_cred_data;
    crypto::Base64EncodeString(cred_data, b64_cred_data);

    std::string encryptedkey = fi.encrypted_key();
    std::string b64_key;
    crypto::Base64EncodeString(encryptedkey, b64_key);

    std::string iv = fi.file_credentials_iv();
    std::string b64_iv;
    crypto::Base64EncodeString(iv, b64_iv);

    std::string query;
    if(IsFileInManifest(fi.filepath())) 
        query += "UPDATE OR REPLACE INTO ";
    else
        query += "INSERT OR REPLACE INTO ";
    query += table_name();
    query += " (filename, filepath, chunkcount, chunkdata, filesize, metapostid,";
    query += " credential_data, postversion, encryptedkey, iv, deleted, folder_post_id, alias_data)";
    query += " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);";

    bool ret = false;
    ret = PrepareStatement(query, error_out);   if(!ret) return ret;
    ret = BindText(1, fi.filename(), error_out);     if(!ret) return ret;
    ret = BindText(2, fi.filepath(), error_out);
    ret = BindInt(3, fi.chunk_count(), error_out);
    ret = BindBlob(4, chunkdata, error_out);
    ret = BindInt(5, fi.file_size(), error_out);
    ret = BindText(6, fi.post_id(), error_out);
    ret = BindText(7, b64_cred_data, error_out);
    ret = BindText(8, fi.post_version(), error_out);
    ret = BindText(9, b64_key, error_out);
    ret = BindBlob(10, b64_iv, error_out);
    ret = BindInt(11, fi.deleted(), error_out);
    ret = BindText(12, fi.folder_post_id(), error_out);
    ret = BindText(13, alias_encoded, error_out);
    ret = StepStatement(error_out);
    ret = FinalizeStatement(error_out);

    return ret;
}

bool InfoTableHandler::RemoveFileInfo(const std::string &filepath) {
    return false;
}

bool InfoTableHandler::IsFileInManifest(const std::string &filepath) {
    return false;
}


}//namespace
