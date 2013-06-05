#include "filetable.h"

namespace attic { 

bool FileTable::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (filename TEXT, filepath TEXT, chunkcount INT,";
    exc += " chunkdata BLOB, filesize INT, metapostid TEXT, credential_data TEXT,";
    exc += " postversion TEXT, encryptedkey BLOB, iv BLOB,";
    exc += " deleted INT, folder_post_id TEXT,";
    exc += " PRIMARY KEY(filepath ASC, folder_post_id ASC, metapostid ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    return ret; 
}

bool FileTable::IsFileInManifest(const std::string &filepath) {
    std::string exc;
    exc += "SELECT EXISTS(SELECT * FROM ";
    exc += table_name();
    exc += " WHERE filepath=\"";
    exc += filepath;
    exc += "\");";
        
    SelectResult res;
    std::string error;
    if(Select(exc ,res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                std::string r = res.results()[0+step];
                if(r == "1")
                    return true;
            }
        }
    }
    return false;
}

bool FileTable::InsertFileInfo(const FileInfo& fi) {
     // Prepare data
    std::string chunkdata;
    fi.GetSerializedChunkData(chunkdata);


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
    query += " credential_data, postversion, encryptedkey, iv, deleted, folder_post_id)";
    query += " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);";

    std::string error;
    bool ret = false;
    ret = PrepareStatement(query, error);           if(!ret) return ret;
    ret = BindText(1, fi.filename(), error);        if(!ret) return ret;
    ret = BindText(2, fi.filepath(), error);        if(!ret) return ret;
    ret = BindInt(3, fi.chunk_count(), error);      if(!ret) return ret;
    ret = BindBlob(4, chunkdata, error);            if(!ret) return ret;
    ret = BindInt(5, fi.file_size(), error);        if(!ret) return ret;
    ret = BindText(6, fi.post_id(), error);         if(!ret) return ret;
    ret = BindText(7, b64_cred_data, error);        if(!ret) return ret;
    ret = BindText(8, fi.post_version(), error);    if(!ret) return ret;
    ret = BindText(9, b64_key, error);              if(!ret) return ret;
    ret = BindBlob(10, b64_iv, error);              if(!ret) return ret;
    ret = BindInt(11, fi.deleted(), error);         if(!ret) return ret;
    ret = BindText(12, fi.folder_post_id(), error); if(!ret) return ret;
    ret = StepStatement(error);                     if(!ret) return ret;
    ret = FinalizeStatement(error);                 if(!ret) return ret;
    return ret;
}

} //namespace

