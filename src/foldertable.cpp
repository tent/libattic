#include "foldertable.h"
#include "logutils.h"

namespace attic { 

bool FolderTable::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (foldername TEXT, post_id TEXT, parent_post_id TEXT, deleted INT,";
    exc += " PRIMARY KEY(foldername ASC, post_id ASC, parent_post_id ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_09214", error);
    return ret; 
}

bool FolderTable::InsertFolderInfo(const std::string& foldername, 
                                   const std::string& folderpostid,
                                   const std::string& parentpostid,
                                   const bool deleted) {
    bool ret = false;
    std::string exc;
    if(!IsFolderInManifest(folderpostid))
        exc += "INSERT OR REPLACE INTO ";
    else
        exc += "REPLACE INTO ";
    exc += table_name();
    exc += " (foldername, post_id, parent_post_id, deleted) VALUES (?,?,?,?);";  
    
    std::string error;
    ret = PrepareStatement(exc, error);     if(!ret) {log::ls("m_240s",error);return ret;}
    ret = BindText(1, foldername, error);   if(!ret) {log::ls("m_241s",error);return ret;}
    ret = BindText(2, folderpostid, error); if(!ret) {log::ls("m_242s",error);return ret;}
    ret = BindText(3, parentpostid, error); if(!ret) {log::ls("m_243s",error);return ret;}
    ret = BindInt(4, deleted, error);       if(!ret) {log::ls("m_244s",error);return ret;}
    ret = StepStatement(error);             if(!ret) {log::ls("m_253s",error);return ret;}
    ret = FinalizeStatement(error);         if(!ret) {log::ls("m_254s",error);return ret;}
               
    return ret;
}

bool FolderTable::RemoveFolderData(const std::string& foldername) {
    return false;
}

bool FolderTable::IsFolderInManifest(const std::string& foldername, 
                                     const std::string& parent_post_id) {
    bool ret = false;
    std::string query;
    query += "SELECT EXISTS(SELECT * FROM ";
    query += table_name();
    query += " WHERE foldername=\"";
    query += foldername;
    query += "\"";
    query += " AND";
    query += " parent_post_id=\"";
    query += parent_post_id;
    query += "\");";
     
    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                std::string r = res.results()[0+step];
                if(r == "1") {
                    ret = true;
                    break;
                }
            }
        }
    }
    else {
        log::LogString("manifest_1901010", error);
    }
    return ret;
}
bool FolderTable::IsFolderInManifest(const std::string& post_id) {
    bool ret = false;
    std::string query;
    query += "SELECT EXISTS(SELECT * FROM ";
    query += table_name();
    query += " WHERE post_id=\"";
    query += post_id;
    query += "\");";
     
    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                std::string r = res.results()[0+step];
                if(r == "1") {
                    ret = true;
                    break;
                }
            }
        }
    }
    else {
        log::LogString("manifest_24893jgs", error);
    }
    return ret;
}

bool FolderTable::QueryForFolder(const std::string& foldername, 
                                 const std::string parent_post_id, 
                                 Folder& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE foldername=\"";
    query += foldername;
    query += "\" AND";
    query += " parent_post_id=\"";
    query += parent_post_id;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                ExtractFolderInfoResults(res, step, out);
        }
        if(step) ret = true;
    }
    else {
        log::LogString("manifest_qqqdg92", error);
    }
    return ret;
}


bool FolderTable::QueryForFolder(const std::string& foldername, Folder& out) { // depricated
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE foldername=\"";
    query += foldername;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                ExtractFolderInfoResults(res, step, out);
        }
        if(step) ret = true;
    }
    else {
        log::LogString("manifest_pmasdg92", error);
    }

    return ret;
}

bool FolderTable::QueryForFolderByPostId(const std::string& post_id, Folder& out) {
    bool ret = false;
    std::string exc;
    exc += "SELECT * FROM ";
    exc += table_name();
    exc += " WHERE post_id=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    SelectResult res;
    if(Select(exc, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                ExtractFolderInfoResults(res, step, out);
        }
        ret = true;
    }
    else {
        log::LogString("manifest_-951-05", error);
    }
    return ret;
}

bool FolderTable::QueryAllFoldersForFolder(const std::string& folderid, FolderList& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE parent_post_id=\"";
    query += folderid;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                Folder folder;
                ExtractFolderInfoResults(res, step, folder);
                out.push_back(folder);
            }
        }
        ret = true;
    }
    else {
        log::LogString("manifest_8915", error);
    }
    return ret;
}
bool FolderTable::set_folder_post_id(const std::string& post_id,
                                     const std::string& new_post_id) {
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET post_id=\"";
    exc += new_post_id;
    exc += "\" WHERE post_id=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)                                       
        log::LogString("manifest_01asgg125", error);
    return ret;
}

bool FolderTable::set_folder_parent_post_id(const std::string& post_id, 
                                            const std::string& parent_post_id) {
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET parent_post_id=\"";
    exc += parent_post_id;
    exc += "\" WHERE post_id=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)                                       
        log::LogString("manifest_01asgg125", error);
    return ret;
}
bool FolderTable::set_foldername(const std::string& post_id, const std::string& foldername) {
    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET foldername=\"";
    exc += foldername;
    exc += "\" WHERE post_id=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)                                       
        log::LogString("manifest_01asgg125", error);
    return ret;
}

bool FolderTable::set_folder_deleted(const std::string& post_id, bool del) {
    char b[256] = {'\0'};
    snprintf(b, 256, "%d", (int)del);

    std::string exc;
    exc += "UPDATE ";
    exc += table_name();
    exc += " SET deleted=\"";
    exc += b;
    exc += "\" WHERE post_id=\"";
    exc += post_id;
    exc += "\";";

    std::string error;
    bool ret = Exec(exc, error);
    if(!ret) {
        error += " | " + exc;
        log::LogString("manifest_01hasdg125", error);
    }
    return ret;
}

bool FolderTable::IsFolderDeleted(const std::string& post_id) {
    bool ret = false;
    std::string query;
    query += "SELECT deleted FROM ";
    query += table_name();
    query += " WHERE post_id=\"";
    query += post_id;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query.c_str(), res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                if(res.results()[0+step])
                    ret = true;
        }
    }
    else {
        log::LogString("manifest_1958sag", error);
    }
    return ret;

}

bool FolderTable::GetFolderPostID(const std::string& foldername, std::string& out) {
    bool ret = false;
    std::string query;
    query += "SELECT post_id FROM ";
    query += table_name();
    query += " WHERE foldername=\"";
    query += foldername;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query.c_str(), res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                out = res.results()[0+step];
        }
        ret = true;
    }
    else {
        log::LogString("manifest_1958sag", error);
    }
    return ret;
}
bool FolderTable::GetFolderPath(const std::string& folder_post_id, std::string& path_out) {
    bool ret = false;
    std::string query;
    query += "SELECT foldername FROM ";
    query += table_name();
    query += " WHERE post_id=\"";
    query += folder_post_id;
    query+= "\"";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                path_out = res.results()[0+step];
        }
        ret = true;
    }
    else {
        log::LogString("manifest_masg91435", error);
    }

    return ret;
}
bool FolderTable::GetFolderId(const std::string& foldername, std::string& out) {
    bool ret = false;
    std::string query;
    query += "SELECT post_id FROM ";
    query += table_name();
    query += " WHERE foldername=\"";
    query += foldername;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                out = res.results()[0+step];
        }
        ret = true;
    }
    else {
        log::LogString("manifest_21014135", error);
    }
    return ret;
}

void FolderTable::ExtractFolderInfoResults(const SelectResult& res, const int step, Folder& out) {
    out.set_foldername(res.results()[0+step]);
    out.set_folder_post_id(res.results()[1+step]);
    out.set_parent_post_id(res.results()[2+step]);
}

} // namespace

