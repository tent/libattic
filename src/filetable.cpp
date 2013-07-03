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
    exc += " PRIMARY KEY(filename ASC, filepath ASC, folder_post_id ASC, metapostid ASC));";
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

bool FileTable::IsFileInManifestWithId(const std::string& post_id) {
    std::string query;
    query += "SELECT EXISTS(SELECT * FROM ";
    query += table_name();
    query += " WHERE metapostid=\"";
    query += post_id;
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

bool FileTable::InsertFileInfo(const std::string& filename, 
                               const std::string& filepath,
                               const unsigned int chunk_count,
                               const std::string& chunk_data,
                               const unsigned int file_size,
                               const std::string& post_id,
                               const std::string& post_version,
                               const std::string& encrypted_key,
                               const std::string& iv,
                               bool deleted,
                               const std::string& folder_post_id,
                               const std::string& plaintext_hash) {
    // Prepare data
    std::string b64_key, b64_iv;
    crypto::Base64EncodeString(encrypted_key, b64_key);
    crypto::Base64EncodeString(iv, b64_iv);

    std::string query;
    if(!IsFileInManifestWithId(post_id))
        query += "INSERT OR REPLACE INTO ";
    else
        query += "REPLACE INTO ";
    query += table_name();
    query += " (filename, filepath, chunkcount, chunkdata, filesize, metapostid,";
    query += " postversion, encryptedkey, iv,";
    query += " deleted, folder_post_id, plaintext_hash)";
    query += " VALUES (?,?,?,?,?,?,?,?,?,?,?,?);";

    std::string error;
    bool ret = false;
    ret = PrepareStatement(query, error);           if(!ret) {log::ls("m_140s",error);return ret;}
    ret = BindText(1, filename, error);             if(!ret) {log::ls("m_141s",error);return ret;}
    ret = BindText(2, filepath, error);             if(!ret) {log::ls("m_142s",error);return ret;}
    ret = BindInt(3, chunk_count, error);           if(!ret) {log::ls("m_143s",error);return ret;}
    ret = BindBlob(4, chunk_data, error);           if(!ret) {log::ls("m_144s",error);return ret;}
    ret = BindInt(5, file_size, error);             if(!ret) {log::ls("m_145s",error);return ret;}
    ret = BindText(6, post_id, error);              if(!ret) {log::ls("m_146s",error);return ret;}
    ret = BindText(7, post_version, error);         if(!ret) {log::ls("m_149s",error);return ret;}
    ret = BindText(8, b64_key, error);              if(!ret) {log::ls("m_140.1s",error);return ret;}
    ret = BindBlob(9, b64_iv, error);               if(!ret) {log::ls("m_150s",error);return ret;}
    ret = BindInt(10, deleted, error);              if(!ret) {log::ls("m_151s",error);return ret;}
    ret = BindText(11, folder_post_id, error);      if(!ret) {log::ls("m_152s",error);return ret;}
    ret = BindText(12, plaintext_hash, error);      if(!ret) {log::ls("m_152.1s",error);return ret;}
    ret = StepStatement(error);                     if(!ret) {log::ls("m_153s",error);return ret;}
    ret = FinalizeStatement(error);                 if(!ret) {log::ls("m_154s",error);return ret;}
    return ret;
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

/*
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
*/

/*
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
*/

/*
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
*/

bool FileTable::set_file_version_for_id(const std::string& post_id, const std::string& version) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE \"";
    exc += table_name();
    exc += "\" SET postversion=\"";
    exc += version;
    exc += "\" WHERE metapostid=\"";
    exc += post_id;
    exc +="\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_102941m129i8412", error);
    return  ret;
}

bool FileTable::set_file_deleted_for_id(const std::string& post_id, const int val) {
    bool ret = false;
    char szDel[256] = {'\0'};
    snprintf(szDel, 256, "%d", val);
    std::string exc;
    exc += "UPDATE \"";
    exc += table_name();
    exc += "\" SET deleted=\"";
    exc += std::string(szDel);
    exc += "\" WHERE metapostid=\"";
    exc += post_id;
    exc +="\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_s1000101s", error);
    return  ret;

}

bool FileTable::set_filepath_for_id(const std::string& post_id, const std::string& filepath) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET filepath=\"";
    exc += filepath;
    exc += "\" WHERE metapostid=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_001918ms5", error);
    return  ret;
}

bool FileTable::set_filename_for_id(const std::string& post_id, const std::string& filename) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET filename=\"";
    exc += filename;
    exc += "\" WHERE metapostid=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_0012588ms5", error);
    return  ret;
}

bool FileTable::set_folder_post_id(const std::string& post_id, const std::string& folder_post_id) {
    bool ret = false;
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET folder_post_id=\"";
    exc += folder_post_id;
    exc += "\" WHERE metapostid=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_010158mgs5", error);
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


bool FileTable::QueryForFile(const std::string& filename, 
                             const std::string& folder_post_id,
                             FileInfo& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE filename=\"";
    query += filename;
    query += "\" AND";
    query += " folder_post_id=\"";
    query += folder_post_id;
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
            ret = set_file_deleted_for_id((*itr).post_id(), 1);
    }
    return ret;
}

} //namespace

