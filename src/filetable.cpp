#include "filetable.h"

#include "logutils.h"

namespace attic { 

bool FileTable::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (filename TEXT, filepath TEXT, chunkcount INT,";
    exc += " chunkdata BLOB, filesize INT, metapostid TEXT,";
    exc += " postversion TEXT, encryptedkey BLOB, iv BLOB,";
    exc += " deleted INT, folder_post_id TEXT, plaintext_hash TEXT,";
    exc += " PRIMARY KEY(filepath ASC, folder_post_id ASC, metapostid ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_09214", error);
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

    std::string local_key = fi.file_credentials().key();
    std::string local_iv = fi.file_credentials().iv();

    std::string encryptedkey = fi.encrypted_key();
    std::string b64_key;
    crypto::Base64EncodeString(encryptedkey, b64_key);

    std::string iv = fi.file_credentials_iv();
    std::string b64_iv;
    crypto::Base64EncodeString(iv, b64_iv);

    std::string query;
    query += "INSERT OR REPLACE INTO ";
    query += table_name();
    query += " (filename, filepath, chunkcount, chunkdata, filesize, metapostid,";
    query += " postversion, encryptedkey, iv,";
    query += " deleted, folder_post_id, plaintext_hash)";
    query += " VALUES (?,?,?,?,?,?,?,?,?,?,?,?);";

    std::string error;
    bool ret = false;
    ret = PrepareStatement(query, error);           if(!ret) {log::ls("m_140s",error);return ret;}
    ret = BindText(1, fi.filename(), error);        if(!ret) {log::ls("m_141s",error);return ret;}
    ret = BindText(2, fi.filepath(), error);        if(!ret) {log::ls("m_142s",error);return ret;}
    ret = BindInt(3, fi.chunk_count(), error);      if(!ret) {log::ls("m_143s",error);return ret;}
    ret = BindBlob(4, chunkdata, error);            if(!ret) {log::ls("m_144s",error);return ret;}
    ret = BindInt(5, fi.file_size(), error);        if(!ret) {log::ls("m_145s",error);return ret;}
    ret = BindText(6, fi.post_id(), error);         if(!ret) {log::ls("m_146s",error);return ret;}
    ret = BindText(7, fi.post_version(), error);    if(!ret) {log::ls("m_149s",error);return ret;}
    ret = BindText(8, b64_key, error);              if(!ret) {log::ls("m_140.1s",error);return ret;}
    ret = BindBlob(9, b64_iv, error);               if(!ret) {log::ls("m_150s",error);return ret;}
    ret = BindInt(10, fi.deleted(), error);         if(!ret) {log::ls("m_151s",error);return ret;}
    ret = BindText(11, fi.folder_post_id(), error); if(!ret) {log::ls("m_152s",error);return ret;}
    ret = BindText(12, fi.plaintext_hash(), error); if(!ret) {log::ls("m_152.1s",error);return ret;}
    ret = StepStatement(error);                     if(!ret) {log::ls("m_153s",error);return ret;}
    ret = FinalizeStatement(error);                 if(!ret) {log::ls("m_154s",error);return ret;}
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
    out.set_post_version(res.results()[6+step]);
    // File Key (Base64 encoded)
    std::string b64_key = res.results()[7+step];
    std::string key;
    crypto::Base64DecodeString(b64_key, key);
    out.set_encrypted_key(key);
    // IV (Base64 encoded)
    std::string b64_iv = res.results()[8+step];
    std::string iv;
    crypto::Base64DecodeString(b64_iv, iv);
    out.set_file_credentials_iv(iv);

    out.set_deleted(atoi(res.results()[9+step]));
    out.set_folder_post_id(res.results()[10+step]);
    out.set_plaintext_hash(res.results()[11+step]);
}

bool FileTable::set_file_post_id(const std::string &filepath, const std::string &id) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE \"";
    exc += table_name();
    exc += "\" SET metapostid=\"";
    exc += id;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc +="\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_svn298140jfs", error);
    return  ret;
}

bool FileTable::set_file_version(const std::string& filepath, const std::string& version) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE \"";
    exc += table_name();
    exc += "\" SET postversion=\"";
    exc += version;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc +="\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_102941m129i8412", error);
    return  ret;
}

bool FileTable::set_file_deleted(const std::string& filepath, const int val) {
    bool ret = false;
    char szDel[256] = {'\0'};
    snprintf(szDel, 256, "%d", val);
    std::string exc;
    exc += "UPDATE \"";
    exc += table_name();
    exc += "\" SET deleted=\"";
    exc += std::string(szDel);
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc +="\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_s1000101s", error);
    return  ret;
}

bool FileTable::set_filepath(const std::string& old_filepath, const std::string& new_filepath) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET filepath=\"";
    exc += new_filepath;
    exc += "\" WHERE filepath=\"";
    exc += old_filepath;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_0nsv9188ds5", error);
    return  ret;
}

bool FileTable::set_filename(const std::string& filepath, const std::string& new_filename) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET filename=\"";
    exc += new_filename;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_0012588ms5", error);
    return  ret;
}

bool FileTable::set_folder_post_id(const std::string& filepath, const std::string& post_id) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET folder_post_id=\"";
    exc += post_id;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_010158mgs5", error);
    return ret;
}

bool FileTable::set_chunk_count(const std::string& filepath, const std::string& chunk_count) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET chunkcount=\"";
    exc += chunk_count;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_01058g55", error);
    return ret;
}

bool FileTable::set_plaintext_hash(const std::string& filepath, const std::string& hash) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET plaintext_hash=\"";
    exc += hash;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_0ma1941155", error);
    return ret;
}

bool FileTable::QueryForFile(const std::string &filepath, FileInfo& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE filepath=\"";
    query += filepath;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                ExtractFileInfoResults(res, step, out);
        }
        ret = true;
    }
    else {
        log::LogString("manifest_8i09255", error);
    }
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
            if(step > 0)
                ExtractFileInfoResults(res, step, out);
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
        for(;itr!= file_list.end(); itr++)
            ret = set_file_deleted((*itr).filepath(), 1);
    }
    return ret;
}

} //namespace

