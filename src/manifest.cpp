#include "manifest.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "utils.h"
#include "crypto.h"

namespace attic {

static const std::string g_infotable("infotable");
static const std::string g_foldertable("foldertable");
static const std::string g_folderentrytable("folderentrytable");

Manifest::Manifest() {
    db_ = NULL;
}

Manifest::~Manifest() {}

void Manifest::SetDirectory(std::string &filepath)  { 
    filepath_ = filepath; 
    utils::CheckUrlAndAppendTrailingSlash(filepath_);
    filepath_ += "manifest";
}

int Manifest::Initialize() {
    return OpenSqliteDb();
}

int Manifest::Shutdown() { 
    int status = CloseSqliteDb();
    std::cout<<" CloseDB status : " << status << std::endl;
    return status;
}

int Manifest::OpenSqliteDb() {
    int status = ret::A_OK;

    int rc = sqlite3_open(filepath_.c_str(), &db_);
    if(rc) {
        // failed
        std::cout<< "Can't open database: " << sqlite3_errmsg(db_);
        std::cout<< " attempted to open : " << filepath_ << std::endl;
        status = ret::A_FAIL_TO_LOAD_MANIFEST;
        db_ = NULL;
    }
    else {
        if(!db_) {
            std::cout << " invlid db instance " << std::endl;
            status = ret::A_FAIL_TO_LOAD_MANIFEST;
        } 
        else { 
            CreateTables();
        }
    }

    return status;
}

int Manifest::CloseSqliteDb() {
    std::cout<<" attempting to close db ... " << std::endl;
    if(db_) {
        int err = sqlite3_close(db_);
        if(err != SQLITE_OK)
            std::cout<<" FAILED TO CLOSE DB " << std::endl;
        db_ = 0;
    }
    return ret::A_OK;
}

bool Manifest::CreateTables() {
    if(!db_)
        return false;

    if(!CreateInfoTable()) {
        std::cout<<" Failed to create infotable..." << std::endl;
        return false;
    }

    if(!CreateFolderTable()) {
        std::cout<<" Failed to create foldertable..." << std::endl;
        return false;
    }

    return true;
}

// TODO :: Perhaps add folderid for queries, can remove folder entry table if we do this
//
bool Manifest::CreateInfoTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += g_infotable;
    exc += " (filename TEXT, filepath TEXT, chunkcount INT,";
    exc += " chunkdata BLOB, filesize INT, metapostid TEXT, credential_data TEXT,";
    exc += " postversion TEXT, encryptedkey BLOB, iv BLOB,";
    exc += " deleted INT, folder_manifest_id TEXT, alias_data TEXT,";
    exc += " PRIMARY KEY(filepath ASC, folder_manifest_id ASC));";

    return PerformQuery(exc);
}

// Folder Table
//  * folder path - RELATIVE path to the folder, if the relative path is empty use <working> 
//    to denote that its whatever Attic folder was set by the app
//  * folderid - random id generate upon creation, folderid is used for various index operations in
//    the folder entry table. We use a folder id to allow the folder to be renamed and its "contents" to remain the same. Without having to update each file individually
//  * folderpostid - id of the attic-folder post containing the metadata
bool Manifest::CreateFolderTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += g_foldertable;
    exc += " (folderpath TEXT, folderid TEXT, folderpostid TEXT,";
    exc += " PRIMARY KEY(folderpath ASC, folderid ASC));";

    return PerformQuery(exc);
}

bool Manifest::PerformQuery(const std::string& query) const {
    if(!db_ || query.empty())
        return false;

    char* szError = NULL;
    int rc = sqlite3_exec( db_, 
                           query.c_str(), 
                           NULL, 
                           NULL,
                           &szError);

    if(rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", szError);
        if(szError) {
            sqlite3_free(szError);
            szError = NULL;
        }
        return false;
    }

    return true;
}

// int sqlite3_get_table(
//                       sqlite3 *db,          /* An open database */
//                       const char *zSql,     /* SQL to be evaluated */
//                       char ***pazResult,    /* Results of the query */
//                       int *prow_,           /* Number of result rows written here */
//                       int *pcol_umn,        /* Number of result columns written here */
//                       char **pzErrmsg       /* Error msg written here */
//                       );
//                      void sqlite3_free_table(char **result);
bool Manifest::PerformSelect(const std::string& select, SelectResult &out) const {
    if(!db_ || select.empty())
        return false;

    char *szError = NULL;
    //char **results;
    //int row_ = 0;
    //int col_ = 0; 
    char *pzErr = NULL;

    int rc = sqlite3_get_table( db_,
                                select.c_str(),       /* SQL to be evaluated */
                                &out.results_,  /* Results of the query */
                                &out.row_,     /* Number of result rows written here */
                                &out.col_,     /* Number of result columns written here */
                                &pzErr         /* Error msg written here */
                               );

    if(pzErr) {
        std::cout << " Perform Select Error : " << pzErr << std::endl;
        return false;
    }

    return true;
}


bool Manifest::IsFileInManifest(const std::string &filepath) {
    std::string pexc;
    pexc += "SELECT * FROM ";
    pexc += g_infotable;
    pexc += " WHERE filepath=\"";
    pexc += filepath;
    pexc += "\";";

    SelectResult res;
    if(PerformSelect(pexc.c_str() ,res)) {
        if(res.row_)
            return true;
    }

    return false;
}

bool Manifest::IsFolderInManifest(const std::string &folderpath) {
    std::string exc;
    exc += "SELECT * FROM ";
    exc += g_foldertable;
    exc += " WHERE folderpath=\"";
    exc += folderpath;
    exc += "\";";
        
    SelectResult res;
    if(PerformSelect(exc.c_str() ,res)) {
        if(res.row_)
            return true;
    }
    return false;
}


bool Manifest::IsFolderInManifestWithID(const std::string& folderid) {
    std::string exc;
    exc += "SELECT * FROM ";
    exc += g_foldertable;
    exc += " WHERE folderid=\"";
    exc += folderid;
    exc += "\";";
        
    SelectResult res;
    if(PerformSelect(exc.c_str() ,res)) {
        if(res.row_)
            return true;
    }

    return false;
}

bool Manifest::QueryForFile(const std::string &filepath, FileInfo& out) {
    char pexc[1024];
    snprintf( pexc,
              1024, 
              "SELECT * FROM %s WHERE filepath=\"%s\";",
              g_infotable.c_str(),
              filepath.c_str()
            );
     
    SelectResult res;
    if(!PerformSelect(pexc, res))
        return false;

    int step = 0;
    for(int i=0; i<res.row_+1; i++) {
        step = i*res.col_;
        //std::cout<< " step : " << step << std::endl;

        /*
        for(int j=0; j<res.col_; j++)
        {
            std::cout << " Results : " << res.results_[j+step] << std::endl;
        }
        */

        if(step > 0) {
            ExtractFileInfoResults(res, step, out);
        }
    }

    if(!step)
        return false;

    return true;
}

int Manifest::QueryAllFiles(FileInfoList& out) {
    int status = ret::A_OK;

    std::string pexc;
    pexc += "SELECT * FROM ";
    pexc += g_infotable;
    pexc += ";";

    SelectResult res;
    if(PerformSelect(pexc.c_str(), res)) {
        /*
        std::cout << " Row count : " << res.row_ << std::endl;
        std::cout << " Col count : " << res.col_ << std::endl;
        */

        int step = 0;
        for(int i=0; i<res.row_+1; i++) {
            step = i*res.col_;
            /*
            for(int j=0; j<res.col_; j++)
            {
                std::cout << " Results : " << res.results_[j+step] << std::endl;
            }
            */

            if(step > 0) {
                FileInfo fi;
                ExtractFileInfoResults(res, step, fi);
                out.push_back(fi);
            }
        }
    }
    else {
        status = ret::A_FAIL_TO_QUERY_MANIFEST;
    }

    return status;
}

int Manifest::QueryAllFilesForFolder(const std::string& folderid, FileInfoList& out) {
    int status = ret::A_OK;
    std::string query;
    query += "SELECT * FROM ";
    query += g_infotable;
    query += " WHERE folder_manifest_id=\"";
    query += folderid;
    query += "\";";
    SelectResult res;
    if(PerformSelect(query.c_str(), res)) {
        int step = 0;
        for(int i=0; i<res.row_+1; i++) {
            step = i*res.col_;
            if(step > 0) {
                FileInfo fi;
                ExtractFileInfoResults(res, step, fi);
                out.push_back(fi);
            }
        }
    }
    else {
        status = ret::A_FAIL_TO_QUERY_MANIFEST;
    }

    return status;
}

void Manifest::ExtractFileInfoResults(const SelectResult& res, const int step, FileInfo& out) {
    out.set_filename(res.results_[0+step]);
    out.set_filepath(res.results_[1+step]);
    out.set_chunk_count(res.results_[2+step]);
    out.LoadSerializedChunkData(res.results_[3+step]);
    out.set_file_size(res.results_[4+step]);
    out.set_post_id(res.results_[5+step]);
    std::string b64_cred_data = res.results_[6+step];
    std::string cred_data;
    crypto::Base64DecodeString(b64_cred_data, cred_data);
    Credentials cred;
    jsn::DeserializeObject(&cred, cred_data);
    out.set_file_credentials(cred);
    out.set_post_version(res.results_[7+step]);
    //out.set_encrypted_key(res.results_[8+step]);
    // File Key (Base64 encoded)
    std::string b64_key = res.results_[8+step];
    std::string key;
    crypto::Base64DecodeString(b64_key, key);
    out.set_encrypted_key(key);
    // IV (Base64 encoded)
    //out.set_file_credentials_iv(res.results_[9+step]);
    std::string b64_iv = res.results_[9+step];
    std::string iv;
    crypto::Base64DecodeString(b64_key, iv);
    out.set_file_credentials_iv(iv);
    out.set_deleted(atoi(res.results_[10+step]));
    out.set_folder_manifest_id(res.results_[11+step]);
    std::string b64_alias = res.results_[12+step];
    std::string alias_data;
    crypto::Base64DecodeString(b64_alias, alias_data);
    out.LoadSerializedAliasData(alias_data);
}

//"CREATE TABLE IF NOT EXISTS %s (filename TEXT, filepath TEXT, chunkcount INT, chunkdata BLOB, filesize INT, metapostid TEXT, credential_data TEXT, postversion INT, key BLOB, PRIMARY KEY(filename ASC));",
              
bool Manifest::InsertFileInfo(const FileInfo& fi) {
    std::cout<<" INSERTING INTO INFO TABLE ! ------------------------------------ " << std::endl;
    if(!db_) {
        std::cout<<"fail db " <<std::endl;
        return false;
    }

    std::string filename = fi.filename();
    std::string filepath = fi.filepath();
    std::string metapostid = fi.post_id();

    std::string cred_data = fi.file_credentials().asString();

    std::string encryptedkey = fi.encrypted_key();
    std::string iv = fi.file_credentials_iv();
    std::string folder_manifest_id = fi.folder_manifest_id();
    
    std::string chunkdata;
    fi.GetSerializedChunkData(chunkdata);


    std::string alias_data;
    fi.GetSerializedAliasData(alias_data);

    std::string alias_encoded;
    crypto::Base64EncodeString(alias_data, alias_encoded);


    std::cout<< " name : " << filename << std::endl;
    std::cout<< " path : " << filepath << std::endl;
    std::cout<< " count : " << fi.chunk_count() << std::endl;
    std::cout<< " filesize : " << fi.file_size() << std::endl;
    std::cout<< " meta id : " << metapostid << std::endl;
    std::cout<< " chunk id : " << cred_data << std::endl;
    std::cout<< " version : " << fi.post_version() << std::endl;
    std::cout<< " chunkdata : " << chunkdata << std::endl;
    std::cout<< " encrypted key : " << encryptedkey << std::endl;
    std::cout<< " iv : " << iv << std::endl;
    std::cout<< " deleted : " << fi.deleted() << std::endl;
    /*
    */

    std::string query;
    query += "INSERT OR REPLACE INTO ";
    query += g_infotable;
    query += " (filename, filepath, chunkcount, chunkdata, filesize, metapostid,";
    query += " credential_data, postversion, encryptedkey, iv, deleted, folder_manifest_id, alias_data)";
    query += " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);";


    // Prepare statement
    sqlite3_stmt* stmt = NULL;
    int ret = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, 0);

    if(ret == SQLITE_OK) {
        if(stmt) {
            ret = sqlite3_bind_text(stmt, 1, filename.c_str(), filename.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("filename Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 2, filepath.c_str(), filepath.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("filepath Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 3, fi.chunk_count());
            if(ret != SQLITE_OK) {
                printf("chunk count Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_blob(stmt, 4, chunkdata.c_str(), chunkdata.size(), SQLITE_TRANSIENT);
            if(ret != SQLITE_OK) {
                printf("chunkdata Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 5, fi.file_size());
            if(ret != SQLITE_OK) {
                printf("filesize Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 6, metapostid.c_str(), metapostid.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("metapostid Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            std::string b64_cred_data;
            crypto::Base64EncodeString(cred_data, b64_cred_data);
            ret = sqlite3_bind_text(stmt, 7, b64_cred_data.c_str(), b64_cred_data.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("credential_data Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 8, fi.post_version().c_str(), fi.post_version().size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("version Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            std::string b64_key;
            crypto::Base64EncodeString(encryptedkey, b64_key);
            ret = sqlite3_bind_blob(stmt, 9, b64_key.c_str(), b64_key.size(), SQLITE_TRANSIENT);
            std::cout<<" BASE 64 KEY ENCRYPTED : " << b64_key << std::endl;

            if(ret != SQLITE_OK) {
                printf("key Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            std::string b64_iv;
            crypto::Base64EncodeString(iv, b64_iv);
            ret = sqlite3_bind_blob(stmt, 10, b64_iv.c_str(), b64_iv.size(), SQLITE_TRANSIENT);
            if(ret != SQLITE_OK) {
                printf(" iv Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 11, fi.deleted());
            if(ret != SQLITE_OK) {
                printf(" deleted Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }
            
            ret = sqlite3_bind_text(stmt, 12, folder_manifest_id.c_str(), folder_manifest_id.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("version Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 13, alias_encoded.c_str(), alias_encoded.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("version Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_step(stmt);
            if(ret != SQLITE_DONE) {
                std::cout<<" return : " << ret << std::endl;
                printf("step Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }

            ret = sqlite3_finalize(stmt);
            if(ret != SQLITE_OK) {
                printf("finalize Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }
        }
        else {
            std::cout<< "Invalid statement" << std::endl;
            printf("Error message: %s\n", sqlite3_errmsg(db_));
            return false;
        }

    }
    else {
        std::cout<< "failed to prepare statement " << std::endl;
        printf("Error message: %s\n", sqlite3_errmsg(db_));
        return false;
    }

    return true;
}

bool Manifest::UpdateAllFileInfoForFolder(const std::string& folderid) { 
    bool ret = false;
    std::string query;
    query += "SELECT * FROM \"";
    query += g_infotable;
    query += "\" WHERE folder_manifest_id=\"";
    query += folderid;
    query += "\";";

    SelectResult res;
    ret = PerformSelect(query.c_str(), res);
    if(ret) {
        int step = 0;
        for(int i=0; i<res.row_+1; i++) {
            step = i*res.col_;
            if(step > 0) {
                FileInfo fi;
                ExtractFileInfoResults(res, step, fi);
                // Update path
                std::string folderid = fi.folder_manifest_id();
                std::string path;
                GetFolderPath(folderid, path);
                // Update alias
                std::string old_path = fi.filepath();
                // Update filepath
                utils::CheckUrlAndAppendTrailingSlash(path);
                path += fi.filename();
                ret = UpdateFilepath(fi.filepath(), path);
                if(ret) {
                    // PushBackAlias
                    ret = PushBackAlias(path, old_path);
                }
            }
        }
    }

    return ret;
}

bool Manifest::PushBackAlias(const std::string& filepath, const std::string& alias) {
    bool ret = false;
    FileInfo fi;
    ret = QueryForFile(filepath, fi);
    if(ret) {
        fi.PushBackAlias(alias);
        ret = InsertFileInfo(fi);
    }

    return ret;
}

bool Manifest::RemoveFileInfo(const std::string &filepath) {
    std::string exc;
    exc += "DELETE FROM \"";
    exc += g_infotable;
    exc += "\"  WHERE filepath=\"";
    exc += filepath;
    exc += "\";";
    return PerformQuery(exc);
}

bool Manifest::UpdateFileVersion(const std::string& filepath, const std::string& version) {
    std::string exc;
    exc += "UPDATE \"";
    exc += g_infotable;
    exc += "\" SET postversion=\"";
    exc += version;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc +="\";";
    return PerformQuery(exc);
}

bool Manifest::UpdateFileDeleted(const std::string& filepath, const int val) {
    char szDel[256] = {'\0'};
    snprintf(szDel, 256, "%d", val);

    std::string exc;
    exc += "UPDATE \"";
    exc += g_infotable;
    exc += "\" SET deleted=\"";
    exc += std::string(szDel);
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc +="\";";
    return PerformQuery(exc);
}

bool Manifest::UpdateFilePostID(const std::string& filepath, const std::string &id) {
    std::string exc;
    exc += "UPDATE \"";
    exc += g_infotable;
    exc += "\" SET metapostid=\"";
    exc += id;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc +="\";";
    return PerformQuery(exc);
}

bool Manifest::UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath) {
    std::string exc;
    exc += "UPDATE ";
    exc += g_infotable;
    exc += " SET filepath=\"";
    exc += new_filepath;
    exc += "\" WHERE filepath=\"";
    exc += old_filepath;
    exc += "\";";

    std::cout<<" PERFORMING QUERY : " << exc << std::endl;
    return PerformQuery(exc);
}

bool Manifest::UpdateFilename(const std::string& filepath, const std::string& new_filename) {
    std::string exc;
    exc += "UPDATE ";
    exc += g_infotable;
    exc += " SET filename=\"";
    exc += new_filename;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc += "\";";

    std::cout<<" PERFORMING QUERY : " << exc << std::endl;
    return PerformQuery(exc);
}

bool Manifest::UpdatePastAlias(const std::string& filepath, const std::string& alias_data) {
    std::string encoded;
    crypto::Base64EncodeString(alias_data, encoded);
    std::string exc;
    exc += "UPDATE ";
    exc += g_infotable;
    exc += " SET alias_data=\"";
    exc += encoded;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc += "\";";

    std::cout<<" PERFORMING QUERY : " << exc << std::endl;
    return PerformQuery(exc);
}

bool Manifest::UpdateFolderPostId(const std::string& folderpath, const std::string& folderpostid) {
    std::string exc;
    exc += "UPDATE ";
    exc += g_foldertable;
    exc += " SET folderpostid=\"";
    exc += folderpostid;
    exc += "\" WHERE folderpath=\"";
    exc += folderpath;
    exc += "\";";
    return PerformQuery(exc);
}

bool Manifest::UpdateFolderPath(const std::string& folderid, const std::string& folderpath) {
    std::string exc;
    exc += "UPDATE ";
    exc += g_foldertable;
    exc += " SET folderpath=\"";
    exc += folderpath;
    exc += "\" WHERE folderid=\"";
    exc += folderid;
    exc += "\";";

    std::cout<<" UPDATING FOLDER PATH : " << exc << std::endl;
    return PerformQuery(exc);
}

bool Manifest::GetFolderPostID(const std::string& folderpath, std::string& out) {
    std::string query;
    query += "SELECT folderpostid FROM ";
    query += g_foldertable;
    query += " WHERE folderpath=\"";
    query += folderpath;
    query += "\";";

    SelectResult res;
    if(!PerformSelect(query.c_str(), res))
        return false;

    int step = 0;
    for(int i=0; i<res.row_+1; i++) {
        step = i*res.col_;
        if(step > 0)
            out = res.results_[0+step];
    }

    if(!step)
        return false;
    return true;
}

bool Manifest::GetFolderPath(const std::string& folder_manifest_id, std::string& path_out) {
    std::string query;
    query += "SELECT folderpath FROM ";
    query += g_foldertable;
    query += " WHERE folderid=\"";
    query += folder_manifest_id;
    query+= "\"";

    SelectResult res;
    if(!PerformSelect(query.c_str(), res))
        return false;

    int step = 0;
    for(int i=0; i<res.row_+1; i++) {
        step = i*res.col_;
        if(step > 0)
            path_out = res.results_[0+step];
    }

    if(!step)
        return false;
    return true;
}

bool Manifest::GetFolderID(const std::string& folderpath, std::string& out) {
    std::string query;
    query += "SELECT folderid FROM ";
    query += g_foldertable;
    query += " WHERE folderpath=\"";
    query += folderpath;
    query += "\";";

    SelectResult res;
    if(!PerformSelect(query.c_str(), res))
        return false;

    int step = 0;
    for(int i=0; i<res.row_+1; i++) {
        step = i*res.col_;
        if(step > 0) { 
            out = res.results_[0+step];
            std::cout<< " OOOOOOUT : " << out << std::endl;
        }

    }

    if(!step)
        return false;
    return true;
}

bool Manifest::InsertFolderInfo(const std::string& folderpath, const std::string& folderpostid) {
    if(!IsFolderInManifest(folderpath)) {
        std::string folderid;
        crypto::GenerateRandomString(folderid);

        while(IsFolderInManifestWithID(folderid)) {
            folderid.clear();
            crypto::GenerateRandomString(folderid);
        }

        std::string exc;
        exc += "INSERT OR REPLACE INTO ";
        exc += g_foldertable;
        exc += " (folderpath, folderid, folderpostid) VALUES (?,?,?);";  

        // Prepare statement
        sqlite3_stmt* stmt = NULL;
        int ret = sqlite3_prepare_v2(db_, exc.c_str(), -1, &stmt, 0);
        if(ret == SQLITE_OK) {
            if(stmt) {
                ret = sqlite3_bind_text(stmt, 1, folderpath.c_str(), folderpath.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(db_));
                    return false;
                }

                ret = sqlite3_bind_text(stmt, 2, folderid.c_str(), folderid.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(db_));
                    return false;
                }

                ret = sqlite3_bind_text(stmt, 3, folderpostid.c_str(), folderpostid.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(db_));
                    return false;
                }

                ret = sqlite3_step(stmt);
                if(ret != SQLITE_DONE) {
                    printf("Error message: %s\n", sqlite3_errmsg(db_));
                    return false;
                }

                ret = sqlite3_finalize(stmt);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(db_));
                    return false;
                }
            }
            else {
                std::cout<< "Invalid statement" << std::endl;
                printf("Error message: %s\n", sqlite3_errmsg(db_));
                return false;
            }
        }
        else {
            std::cout<< "failed to prepare statement " << std::endl;
            printf("Error message: %s\n", sqlite3_errmsg(db_));
            return false;
        }
        return true;
    }
    else {
        std::cout<<" trying to insert to folder that already exists" << std::endl;
    }

    return false;
}

bool Manifest::RemoveFolderData(const std::string& folderpath)
{

    return false;
}

}//namespace
