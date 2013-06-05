#include "filetable.h"

#include "logutils.h"

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
    if(!ret) {
        log::LogString("manifest_09214", error);
    }
    return ret; 
}

bool FileTable::IsFileInManifest(const std::string &filepath) {
    std::string query;
    query += "SELECT EXISTS(SELECT * FROM ";
    query += table_name();
    query += " WHERE filepath=\"";
    query += filepath;
    query += "\");";
        
    SelectResult res;
    std::string error;
    if(Select(query ,res, error)) {
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

bool FileTable::RemoveFileInfo(const std::string &filepath) {
    bool ret = false;
    return  ret;
}

void FileTable::ExtractFileInfoResults(const SelectResult& res, const int step, FileInfo& out) {
/* Note : Remember the manifest is a point of data contention, any string set here NEEDS avoid 
 *        copy on write, make sure the string is EXCLICITLY COPIED out of here whether this is 
 *        by the append method or some other means, just make sure to avoid copy on write and 
 *        any other operation that possibly uses references in the underlying implementation
 *        instead of directly copying the buffer.
 */
    out.set_filename(res.results()[0+step]);
    out.set_filepath(res.results()[1+step]);
    out.set_chunk_count(res.results()[2+step]);
    out.LoadSerializedChunkData(res.results()[3+step]);
    out.set_file_size(res.results()[4+step]);
    out.set_post_id(res.results()[5+step]);
    std::string b64_cred_data = res.results()[6+step];
    std::string cred_data;
    crypto::Base64DecodeString(b64_cred_data, cred_data);
    Credentials cred;
    jsn::DeserializeObject(&cred, cred_data);
    out.set_file_credentials(cred);
    out.set_post_version(res.results()[7+step]);
    //out.set_encrypted_key(res.results()[8+step]);
    // File Key (Base64 encoded)
    std::string b64_key = res.results()[8+step];
    std::string key;
    crypto::Base64DecodeString(b64_key, key);
    out.set_encrypted_key(key);
    // IV (Base64 encoded)
    //out.set_file_credentials_iv(res.results()[9+step]);
    std::string b64_iv = res.results()[9+step];
    std::string iv;
    crypto::Base64DecodeString(b64_iv, iv);
    out.set_file_credentials_iv(iv);
    out.set_deleted(atoi(res.results()[10+step]));
    out.set_folder_post_id(res.results()[11+step]);
}

bool FileTable::set_file_post_id(const std::string &filename, const std::string &id) {
    bool ret = false;
    return  ret;
}

bool FileTable::set_file_version(const std::string& filepath, const std::string& version) {
    bool ret = false;
    return  ret;
}

bool FileTable::set_file_deleted(const std::string& filepath, const int val) {
    bool ret = false;
    return  ret;
}

bool FileTable::set_filepath(const std::string& old_filepath, const std::string& new_filepath) {
    bool ret = false;
    return  ret;
}

bool FileTable::set_filename(const std::string& filepath, const std::string& new_filename) {
    bool ret = false;
    return  ret;
}

bool FileTable::set_folder_post_id(const std::string& filepath, const std::string& post_id) {
    bool ret = false;
    return  ret;
}

bool FileTable::set_chunk_count(const std::string& filepath, const std::string& chunk_count) {
    bool ret = false;
    return  ret;
}

bool FileTable::QueryForFile(const std::string &filepath, FileInfo& out) {
    bool ret = false;
    return  ret;
}

bool FileTable::QueryForFileByPostId(const std::string& post_id, FileInfo& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE metapostid=\"";
    query += post_id;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                ExtractFileInfoResults(res, step, out);
            }
        }
        ret = true;
    }
    else {
        log::LogString("manifest_1458715", error);
    }
    return  ret;
}

bool FileTable::QueryAllFiles(FileInfoList& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += ";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                FileInfo fi;
                ExtractFileInfoResults(res, step, fi);
                out.push_back(fi);
            }
        }
        ret = true;
    }
    else {
        log::LogString("manifest_-58934", error);
    }

    return ret;
}

bool FileTable::QueryAllFilesForFolder(const std::string& folderid, FileInfoList& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE folder_post_id=\"";
    query += folderid;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                FileInfo fi;
                ExtractFileInfoResults(res, step, fi);
                out.push_back(fi);
            }
        }
        ret = true;
    }
    else {
        log::LogString("manifest_46518", error);
    }
    return ret;
}

bool FileTable::MarkAllFilesDeletedInFolder(const std::string& folderid) {
    bool ret = false;
    FileInfoList file_list;
    if(QueryAllFilesForFolder(folderid, file_list) == ret::A_OK) {
        FileInfoList::iterator itr = file_list.begin();
        for(;itr!= file_list.end(); itr++) {
            ret = set_file_deleted((*itr).filepath(), 1);
        }
    }
    return ret;
}

} //namespace

