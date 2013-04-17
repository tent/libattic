#include "manifest.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

#include "utils.h"
#include "crypto.h"

namespace attic {

static const std::string g_infotable("infotable");
static const std::string g_foldertable("foldertable");
static const std::string g_folderentrytable("folderentrytable");

Manifest::Manifest() {
    m_pDb = NULL;
}

Manifest::~Manifest() {}

void Manifest::SetDirectory(std::string &filepath)  { 
    m_Filepath = filepath; 
    utils::CheckUrlAndAppendTrailingSlash(m_Filepath);
    m_Filepath += "manifest";
}
/*
 * Order to write out (and read in)
 * Manifest Header
 * - number of entries (unsigned long) 
 *
 * Entry
 * - Filename (str)
 * - Filepath (str)
 * - ChunkCount (unsigned int)
 * - ChunkData (str)
 * - FileSize (unsigned int)
 * - MetaPostID (str)
 * - ChunkPostID (str)
 * - PostVersion (unsigned int)
 * - Encrypted Key (blob) // Encrytped
 * - Iv (blob) // not encrypted
 */

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

    int rc = sqlite3_open(m_Filepath.c_str(), &m_pDb);
    if(rc) {
        // failed
        std::cout<< "Can't open database: " << sqlite3_errmsg(m_pDb);
        std::cout<< " attempted to open : " << m_Filepath << std::endl;
        status = ret::A_FAIL_TO_LOAD_MANIFEST;
        m_pDb = NULL;
    }
    else {
        if(!m_pDb) {
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
    if(m_pDb) {
        int err = sqlite3_close(m_pDb);
        if(err != SQLITE_OK)
            std::cout<<" FAILED TO CLOSE DB " << std::endl;
        m_pDb = 0;
    }
    return ret::A_OK;
}

bool Manifest::CreateTables() {
    if(!m_pDb)
        return false;

    if(!CreateInfoTable()) {
        std::cout<<" Failed to create infotable..." << std::endl;
        return false;
    }

    if(!CreateFolderTable()) {
        std::cout<<" Failed to create foldertable..." << std::endl;
        return false;
    }

    if(!CreateFolderEntryTable()) {
        std::cout<<" Failed to create folderentrytable..."<< std::endl;
        return false;
    }


    return true;
}

bool Manifest::CreateInfoTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += g_infotable;
    exc += " (filename TEXT, filepath TEXT, chunkcount INT,";
    exc += " chunkdata BLOB, filesize INT, metapostid TEXT, chunkpostid TEXT,";
    exc += " postversion TEXT, encryptedkey BLOB, iv BLOB,";
    exc += " deleted INT, PRIMARY KEY(filepath ASC));";

    return PerformQuery(exc);
}

// Folder Table
//  * folder path - RELATIVE path to the folder, if the relative path is empty use <working> 
//    to denote that its whatever Attic folder was set by the app
//  * folderid - random id generate upon creation, folderid is used for various index operations in
//    the folder entry table.
//  * folderpostid - id of the attic-folder post containing the metadata
bool Manifest::CreateFolderTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += g_foldertable;
    exc += " (folderpath TEXT, folderid TEXT, folderpostid TEXT,";
    exc += " PRIMARY KEY(folderpath ASC));";

    return PerformQuery(exc);
}

// Folder Entry Table
//  * folderid - id of the folder this file entry belongs to, see folder table
//  * metapostid - id of the corresponding metadata post for the file
//  * type - file or folder
//  * path - RELATIVE filepath, in relation to the working directory, if just filename assume
//    top level directory ie: <working>/filename.txt
bool Manifest::CreateFolderEntryTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += g_folderentrytable;
    exc += " (folderid TEXT, metapostid TEXT, type TEXT, filepath TEXT,";
    exc += " PRIMARY KEY(folderid ASC, filepath ASC));";

    return PerformQuery(exc);
}

bool Manifest::PerformQuery(const std::string& query) const {
    if(!m_pDb || query.empty())
        return false;

    char* szError = NULL;
    int rc = sqlite3_exec( m_pDb, 
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
//                       int *pnRow,           /* Number of result rows written here */
//                       int *pnColumn,        /* Number of result columns written here */
//                       char **pzErrmsg       /* Error msg written here */
//                       );
//                      void sqlite3_free_table(char **result);
bool Manifest::PerformSelect(const std::string& select, SelectResult &out) const {
    if(!m_pDb || select.empty())
        return false;

    char *szError = NULL;
    //char **results;
    //int nRow = 0;
    //int nCol = 0; 
    char *pzErr = NULL;

    int rc = sqlite3_get_table( m_pDb,
                                select.c_str(),       /* SQL to be evaluated */
                                &out.results,  /* Results of the query */
                                &out.nRow,     /* Number of result rows written here */
                                &out.nCol,     /* Number of result columns written here */
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
        if(res.nRow)
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
        if(res.nRow)
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
        if(res.nRow)
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
    for(int i=0; i<res.nRow+1; i++) {
        step = i*res.nCol;
        //std::cout<< " step : " << step << std::endl;

        /*
        for(int j=0; j<res.nCol; j++)
        {
            std::cout << " Results : " << res.results[j+step] << std::endl;
        }
        */

        if(step > 0) {
            out.set_filename(res.results[0+step]);
            out.set_filepath(res.results[1+step]);
            out.set_chunk_count(res.results[2+step]);
            out.LoadSerializedChunkData(res.results[3+step]);
            out.set_file_size(res.results[4+step]);
            out.set_post_id(res.results[5+step]);
            out.set_chunk_post_id(res.results[6+step]);
            out.set_post_version(res.results[7+step]);
            //out.set_encrypted_key(res.results[8+step]);
            // File Key (Base64 encoded)
            std::string b64_key = res.results[8+step];
            std::string key;
            crypto::Base64DecodeString(b64_key, key);
            out.set_encrypted_key(key);
            // IV (Base64 encoded)
            //out.set_file_credentials_iv(res.results[9+step]);
            std::string b64_iv = res.results[9+step];
            std::string iv;
            crypto::Base64DecodeString(b64_key, iv);
            out.set_file_credentials_iv(iv);
            //
            out.set_deleted(res.results[10+step]);
        }
    }

    sqlite3_free_table(res.results);

    if(!step)
        return false;

    return true;
}

int Manifest::QueryAllFiles(std::vector<FileInfo>& out) {
    int status = ret::A_OK;

    std::string pexc;
    pexc += "SELECT * FROM ";
    pexc += g_infotable;
    pexc += ";";

    SelectResult res;
    
    if(PerformSelect(pexc.c_str(), res)) {

        /*
        std::cout << " Row count : " << res.nRow << std::endl;
        std::cout << " Col count : " << res.nCol << std::endl;
        */

        int step = 0;
        for(int i=0; i<res.nRow+1; i++) {
            step = i*res.nCol;
            /*
            for(int j=0; j<res.nCol; j++)
            {
                std::cout << " Results : " << res.results[j+step] << std::endl;
            }
            */

            if(step > 0) {
                FileInfo fi;
                fi.set_filename(res.results[0+step]);
                fi.set_filepath(res.results[1+step]);
                fi.set_chunk_count(res.results[2+step]);
                fi.LoadSerializedChunkData(res.results[3+step]);
                fi.set_file_size(res.results[4+step]);
                fi.set_post_id(res.results[5+step]);
                fi.set_chunk_post_id(res.results[6+step]);
                fi.set_post_version(res.results[7+step]);
                //fi.set_encrypted_key(res.results[8+step]);
                std::string b64_key = res.results[8+step];
                std::string key;
                crypto::Base64DecodeString(b64_key, key);
                fi.set_encrypted_key(key);
                //fi.set_file_credentials_iv(res.results[9+step]);
                std::string b64_iv = res.results[9+step];
                std::string iv;
                crypto::Base64DecodeString(b64_iv, iv);
                fi.set_file_credentials_iv(iv);
                //
                fi.set_deleted(res.results[10+step]);

                out.push_back(fi);
            }
        }

        sqlite3_free_table(res.results);
    }
    else {
        status = ret::A_FAIL_TO_QUERY_MANIFEST;
    }

    return status;
}

//"CREATE TABLE IF NOT EXISTS %s (filename TEXT, filepath TEXT, chunkcount INT, chunkdata BLOB, filesize INT, metapostid TEXT, chunkpostid TEXT, postversion INT, key BLOB, PRIMARY KEY(filename ASC));",
              
bool Manifest::InsertFileInfo(const FileInfo& fi) {
    std::cout<<" INSERTING INTO INFO TABLE ! ------------------------------------ " << std::endl;
    if(!m_pDb) {
        std::cout<<"fail db " <<std::endl;
        return false;
    }

    std::string filename = fi.filename();
    std::string filepath = fi.filepath();
    std::string metapostid = fi.post_id();
    std::string chunkpostid = fi.chunk_post_id();
    std::string encryptedkey = fi.encrypted_key();
    std::string iv = fi.file_credentials_iv();
    
    std::string chunkdata;
    fi.GetSerializedChunkData(chunkdata);

    /*
    std::cout<< " name : " << filename << std::endl;
    std::cout<< " path : " << filepath << std::endl;
    std::cout<< " count : " << fi.GetChunkCount() << std::endl;
    std::cout<< " filesize : " << fi.GetFileSize() << std::endl;
    std::cout<< " meta id : " << metapostid << std::endl;
    std::cout<< " chunk id : " << chunkpostid << std::endl;
    std::cout<< " version : " << fi.GetPostVersion() << std::endl;
    std::cout<< " chunkdata : " << chunkdata << std::endl;
    std::cout<< " encrypted key : " << encryptedkey << std::endl;
    std::cout<< " iv : " << iv << std::endl;
    std::cout<< " deleted : " << fi.GetDeleted() << std::endl;
    */

    std::string query;
    query += "INSERT OR REPLACE INTO ";
    query += g_infotable;
    query += " (filename, filepath, chunkcount, chunkdata, filesize, metapostid,";
    query += " chunkpostid, postversion, encryptedkey, iv, deleted)";
    query += " VALUES (?,?,?,?,?,?,?,?,?,?,?);";


    // Prepare statement
    sqlite3_stmt* stmt = NULL;
    int ret = sqlite3_prepare_v2(m_pDb, query.c_str(), -1, &stmt, 0);

    if(ret == SQLITE_OK) {
        if(stmt) {
            ret = sqlite3_bind_text(stmt, 1, filename.c_str(), filename.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("filename Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 2, filepath.c_str(), filepath.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("filepath Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 3, fi.chunk_count());
            if(ret != SQLITE_OK) {
                printf("chunk count Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_blob(stmt, 4, chunkdata.c_str(), chunkdata.size(), SQLITE_TRANSIENT);
            if(ret != SQLITE_OK) {
                printf("chunkdata Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 5, fi.file_size());
            if(ret != SQLITE_OK) {
                printf("filesize Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 6, metapostid.c_str(), metapostid.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("metapostid Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 7, chunkpostid.c_str(), chunkpostid.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("chunkpostid Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 8, fi.post_version().c_str(), fi.post_version().size(), SQLITE_STATIC);
            if(ret != SQLITE_OK) {
                printf("version Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            std::string b64_key;
            crypto::Base64EncodeString(encryptedkey, b64_key);
            ret = sqlite3_bind_blob(stmt, 9, b64_key.c_str(), b64_key.size(), SQLITE_TRANSIENT);

            if(ret != SQLITE_OK) {
                printf("key Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            std::string b64_iv;
            crypto::Base64EncodeString(iv, b64_iv);
            ret = sqlite3_bind_blob(stmt, 10, b64_iv.c_str(), b64_iv.size(), SQLITE_TRANSIENT);
            if(ret != SQLITE_OK) {
                printf(" iv Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 11, fi.deleted());
            if(ret != SQLITE_OK) {
                printf(" deleted Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_step(stmt);
            if(ret != SQLITE_DONE) {
                std::cout<<" return : " << ret << std::endl;
                printf("step Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_finalize(stmt);
            if(ret != SQLITE_OK) {
                printf("finalize Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }
        }
        else {
            std::cout<< "Invalid statement" << std::endl;
            printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
            return false;
        }

    }
    else {
        std::cout<< "failed to prepare statement " << std::endl;
        printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
        return false;
    }

    return true;
}

bool Manifest::RemoveFileInfo(const std::string &filepath)
{
    std::string exc;
    exc += "DELETE FROM \"";
    exc += g_infotable;
    exc += "\"  WHERE filepath=\"";
    exc += filepath;
    exc += "\";";
    return PerformQuery(exc);
}

bool Manifest::UpdateFileVersion(const std::string& filepath, const std::string& version)
{
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

bool Manifest::UpdateFileDeleted(const std::string& filepath, const int val)
{
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

bool Manifest::UpdateFilePostID(const std::string& filepath, const std::string &id)
{
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

bool Manifest::UpdateFileChunkPostID(const std::string &filepath, const std::string &id) {
    std::string exc;
    exc += "UPDATE ";
    exc += g_infotable.c_str();
    exc += " SET chunkpostid=\"";
    exc += id.c_str();
    exc += "\" WHERE filepath=\"";
    exc += filepath.c_str();
    exc += "\";";
    return PerformQuery(exc);
}

bool Manifest::UpdateFilepath(const std::string& old_filepath, const std::string& new_filepath) {
    std::string exc;
    exc += "UPDATE ";
    exc += g_infotable.c_str();
    exc += " SET filepath=\"";
    exc += new_filepath.c_str();
    exc += "\" WHERE filepath=\"";
    exc += old_filepath.c_str();
    exc += "\";";

    std::cout<<" PERFORMING QUERY : " << exc << std::endl;
    return PerformQuery(exc);
}

bool Manifest::UpdateFilename(const std::string& filepath, const std::string& new_filename) {
    std::string exc;
    exc += "UPDATE ";
    exc += g_infotable.c_str();
    exc += " SET filename=\"";
    exc += new_filename.c_str();
    exc += "\" WHERE filepath=\"";
    exc += filepath.c_str();
    exc += "\";";

    std::cout<<" PERFORMING QUERY : " << exc << std::endl;
    return PerformQuery(exc);
}

bool Manifest::UpdateFolderPostID(const std::string& folderpath, const std::string& folderpostid) {
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





bool Manifest::GetFolderPostID(const std::string& folderpath, std::string& out)
{
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
    for(int i=0; i<res.nRow+1; i++) {
        step = i*res.nCol;
        if(step > 0)
            out = res.results[0+step];
    }

    sqlite3_free_table(res.results);
    if(!step)
        return false;
    return true;
}

bool Manifest::GetFolderID(const std::string& folderpath, std::string& out)
{
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
    for(int i=0; i<res.nRow+1; i++) {
        step = i*res.nCol;
        if(step > 0)
            out = res.results[0+step];
    }

    sqlite3_free_table(res.results);
    if(!step)
        return false;
    return true;
}

bool Manifest::InsertFolderInfo(const std::string& folderpath, const std::string& folderpostid)
{
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
        int ret = sqlite3_prepare_v2(m_pDb, exc.c_str(), -1, &stmt, 0);
        if(ret == SQLITE_OK) {
            if(stmt) {
                ret = sqlite3_bind_text(stmt, 1, folderpath.c_str(), folderpath.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_bind_text(stmt, 2, folderid.c_str(), folderid.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_bind_text(stmt, 3, folderpostid.c_str(), folderpostid.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_step(stmt);
                if(ret != SQLITE_DONE) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_finalize(stmt);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }
            }
            else {
                std::cout<< "Invalid statement" << std::endl;
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }
        }
        else {
            std::cout<< "failed to prepare statement " << std::endl;
            printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
            return false;
        }
        return true;
    }
    else {
        std::cout<<" trying to insert to folder that already exists" << std::endl;
    }

    return false;
}

//" (folderid TEXT, metapostid TEXT, type TEXT, filepath TEXT,";
bool Manifest::InsertFolderEnrty( const std::string& folderid, 
                                  const std::string& metapostid, 
                                  const std::string& type,
                                  const std::string& filepath)
{
    if(!IsFolderEntryInManifest(filepath)) {
        std::string exc;
        exc += "INSERT OR REPLACE INTO ";
        exc += g_folderentrytable;
        exc += " (folderid, metapostid, type, filepath) VALUES (?,?,?,?);";  

        // Prepare statement
        sqlite3_stmt* stmt = NULL;
        int ret = sqlite3_prepare_v2(m_pDb, exc.c_str(), -1, &stmt, 0);
        if(ret == SQLITE_OK) {
            if(stmt) {
                ret = sqlite3_bind_text(stmt, 1, folderid.c_str(), folderid.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_bind_text(stmt, 2, metapostid.c_str(), metapostid.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_bind_text(stmt, 3, type.c_str(), type.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_bind_text(stmt, 4, filepath.c_str(), filepath.size(), SQLITE_STATIC);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_step(stmt);
                if(ret != SQLITE_DONE) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }

                ret = sqlite3_finalize(stmt);
                if(ret != SQLITE_OK) {
                    printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                    return false;
                }
            }
            else {
                std::cout<< "Invalid statement" << std::endl;
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }
        }
        else {
            std::cout<< "failed to prepare statement " << std::endl;
            printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
            return false;
        }
        return true;
    }
    else {
        std::cout<<" trying to insert a folder entry that already exists " << std::endl;
    }

    return false;
}

bool Manifest::IsFolderEntryInManifest(const std::string& filepath)
{
    std::string exc;
    exc += "SELECT * FROM ";
    exc += g_folderentrytable;
    exc += " WHERE filepath=\"";
    exc += filepath;
    exc += "\";";
        
    SelectResult res;
    if(PerformSelect(exc.c_str() ,res)) {
        if(res.nRow)
            return true;
    }
    return false;
}

bool Manifest::SetFolderEntryMetapostID(const std::string& filepath, const std::string& metapostid)
{
    std::string exc;
    exc += "UPDATE ";
    exc += g_folderentrytable;
    exc += " SET metapostid=\"";
    exc += metapostid;
    exc += "\" WHERE filepath=\"";
    exc += filepath;
    exc += "\";";
    return PerformQuery(exc);
}

bool Manifest::GetFolderEntryMetapostID(const std::string& filepath, std::string& out)
{
    std::string query;
    query += "SELECT metapostid FROM ";
    query += g_folderentrytable;
    query += " WHERE filepath=\"";
    query += filepath;
    query += "\";";

    SelectResult res;
    if(!PerformSelect(query.c_str(), res))
        return false;

    int step = 0;
    for(int i=0; i<res.nRow+1; i++) {
        step = i*res.nCol;
        if(step > 0)
            out = res.results[0+step];
    }

    sqlite3_free_table(res.results);
    if(!step)
        return false;
    return true;

}

    //exc += " (folderid TEXT, metapostid TEXT, type TEXT, filepath TEXT,";
bool Manifest::GetFolderEntries(const std::string& folderid, std::vector<FolderEntry>& out)
{
    std::string query;
    query += "SELECT * FROM ";
    query += g_folderentrytable;
    query += " WHERE folderid=\"";
    query += folderid;
    query += "\";";

    SelectResult res;
    if(!PerformSelect(query, res))
        return false;

    //std::cout<<" query start " << std::endl;
    int step = 0;
    for(int i=0; i<res.nRow+1; i++) {
        step = i*res.nCol;
        
        for(int j=0; j<res.nCol; j++)
            std::cout << " Results : " << res.results[j+step] << std::endl;

        if(step > 0) {
            FolderEntry fe;
            // ignore folder id
            fe.SetPostID(res.results[1+step]);
            fe.SetType(res.results[2+step]);
            fe.SetPath(res.results[3+step]);

            out.push_back(fe);
        }
    }

    //std::cout<<" query done " << std::endl;
    sqlite3_free_table(res.results);

    if(!step)
        return false;

    return true;


}

bool Manifest::RemoveFolderData(const std::string& folderpath)
{

    return false;
}

}//namespace
