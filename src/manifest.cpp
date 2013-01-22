#include "manifest.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

#include "fileinfo.h"
#include "utils.h"


static const std::string g_infotable("infotable");
static const std::string g_foldertable("foldertable");
static const std::string g_metatable("metatable");

Manifest::Manifest()
{
    // Set the default filename
    m_Filename.append("manifest._mn");
    m_EntryCount = 0;
    m_VersionNumber = 0;
    m_pDb = NULL;
    m_Dirty = false;
}

Manifest::~Manifest()
{

}
/*
 * Order to write out (and read in)
 * Manifest Header
 * - number of entries (unsigned long) 
 *
 * Entry
 * - Filename (str)
 * - Filepath (str)
 * - ChunkName (str)
 * - ChunkCount (unsigned int)
 * - ChunkData (str)
 * - FileSize (unsigned int)
 * - MetaPostID (str)
 * - ChunkPostID (str)
 * - PostVersion (unsigned int)
 * - Key (blob)
 */
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{

    std::cout << " callback hit ... " << std::endl;
    int i;

    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");

    return 0;

}

void Manifest::Initialize()
{
    OpenSqliteDb();
}

void Manifest::Shutdown()
{
    CloseSqliteDb();
}

void Manifest::OpenSqliteDb()
{
    std::cout<<" attempting to open : " << m_Filepath << std::endl;
    int rc = sqlite3_open(m_Filepath.c_str(), &m_pDb);

    if( rc )
    {
        // failed
        std::cout<< "Can't open database: " << sqlite3_errmsg(m_pDb);
    }
    else
    {
        if(!m_pDb)
        {
            std::cout << " invlid db instance " << std::endl;
            return;
        }

        CreateTables();
    }
}

void Manifest::CloseSqliteDb()
{
    if(m_pDb)
    {
        sqlite3_close(m_pDb);
        m_pDb = 0;
    }
}

bool Manifest::CreateTables()
{
    if(!m_pDb)
        return false;

    if(!CreateInfoTable())
    {
        std::cout<<" Failed to create infotable..." << std::endl;
        return false;
    }

    if(!CreateMetaTable())
    {
        std::cout<<" Failed to create metatable..." << std::endl;
        return false;
    }

    if(!CreateFolderTable())
    {
        std::cout<<" Failed to create foldertable..." << std::endl;
        return false;
    }

    return true;
}

bool Manifest::CreateInfoTable()
{
    char pexc[1024];
    snprintf( pexc,
              1024,
              "CREATE TABLE IF NOT EXISTS %s (filename TEXT, filepath TEXT, chunkname TEXT, chunkcount INT, chunkdata BLOB, filesize INT, metapostid TEXT, chunkpostid TEXT, postversion INT, key BLOB, PRIMARY KEY(filename ASC));",
              g_infotable.c_str()
            );
     
    /*
    snprintf( pexc,
              1024,
              "CREATE TABLE IF NOT EXISTS %s (filename TEXT, filepath TEXT, chunkname TEXT, chunkcount INT, filesize INT, postid TEXT, postversion INT, key BLOB, iv BLOB, PRIMARY KEY(filename ASC));",
              g_infotable.c_str()
            );
            */

    return PerformQuery(pexc);
}

bool Manifest::CreateMetaTable()
{
    char pexc[1024];

    snprintf( pexc,
              1024,
              "CREATE TABLE IF NOT EXISTS %s (key TEXT, value TEXT, PRIMARY KEY(key ASC));",
              g_metatable.c_str()
            );

    return PerformQuery(pexc);
}

bool Manifest::CreateFolderTable()
{
    std::string pexc;
    pexc += "CREATE TABLE IF NOT EXISTS ";
    pexc += g_foldertable;
    pexc += " (name TEXT, path TEXT, children TEXT, postid TEXT, PRIMARY KEY(path ASC));";

    return PerformQuery(pexc.c_str());
}

bool Manifest::PerformQuery(const char* pQuery) const
{
    if(!m_pDb || !pQuery)
        return false;

    char* szError = NULL;
    int rc = sqlite3_exec( m_pDb, 
                           pQuery, 
                           NULL, 
                           NULL,
                           &szError);

    if(rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", szError);
        if(szError)
        {
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
bool Manifest::PerformSelect(const char* pSelect, SelectResult &out) const
{
    if(!m_pDb || !pSelect)
        return false;

    char *szError = NULL;
    //char **results;
    //int nRow = 0;
    //int nCol = 0; 
    char *pzErr = NULL;

    int rc = sqlite3_get_table( m_pDb,
                                pSelect,       /* SQL to be evaluated */
                                &out.results,  /* Results of the query */
                                &out.nRow,     /* Number of result rows written here */
                                &out.nCol,     /* Number of result columns written here */
                                &pzErr         /* Error msg written here */
                               );

    if(pzErr)
    {
        std::cout << " Perform Select Error : " << pzErr << std::endl;
        return false;
    }

    return true;
}


bool Manifest::QueryForFileExistence(const std::string& filename)
{
    char pexc[1024];

    snprintf( pexc,
              1024, 
              "SELECT * FROM %s WHERE filename=\"%s\";",
              g_infotable.c_str(),
              filename.c_str()
            );
     
    std::cout<< " EXECING : " << pexc << std::endl;
    
    SelectResult res;
    if(PerformSelect(pexc,res))
    {
        if(res.nRow)
            return true;
    }

    return false;
}

void Manifest::QueryForMetaPostID(std::string &out) const
{
   char pexc[1024];

    snprintf( pexc,
              1024, 
              "SELECT * FROM %s WHERE key=\"%s\";",
              g_metatable.c_str(),
              "post" 
            );
     
    std::cout<< " EXECING : " << pexc << std::endl;

    SelectResult res;
    if(!PerformSelect(pexc, res))
    {
        // Most likely doesn't exist, if not create it and set to 1
        std::cout<<"Doesn't exist..."<<std::endl;
    }
    else 
    {
        if(res.nRow == 0)
        {
            std::cout<<" no entry..."<<std::endl;
        }
        else
        {
            // Extract version number
            int step = 0;
            for(int i=0; i < res.nRow+1; i++)
            {
                step = i*res.nCol;
                /*
                for(int j=0; j < res.nCol; j++)
                {
                    std::cout<< "\t" << res.results[j + step];
                }
                */

                if(step > 0)
                {
                    out = res.results[1 + step];
                    break;
                }
            }

            std::cout<<" POST : " << out << std::endl;
        }
    }


}

unsigned int Manifest::QueryForVersion() const
{
    char pexc[1024];

    snprintf( pexc,
              1024, 
              "SELECT * FROM %s WHERE key=\"%s\";",
              g_metatable.c_str(),
              "version" 
            );
     
    std::cout<< " EXECING : " << pexc << std::endl;

    SelectResult res;
    unsigned int version = 1;
    if(!PerformSelect(pexc, res))
    {
        // Most likely doesn't exist, if not create it and set to 1
        std::cout<<"Doesn't exist..."<<std::endl;
        version = 0;
    }
    else 
    {
        std::string hold;
        if(res.nRow == 0)
        {
            std::cout<<" no entry, set default"<<std::endl;
            InsertVersionNumber(1);
        }
        else
        {
            // Extract version number
            int step = 0;
            for(int i=0; i < res.nRow+1; i++)
            {
                step = i*res.nCol;
                /*
                for(int j=0; j < res.nCol; j++)
                {
                    std::cout<< "\t" << res.results[j + step];
                }
                */

                if(step > 0)
                {
                    hold = res.results[1 + step];
                    break;
                }
            }

            version = atoi(hold.c_str());
            //std::cout<<" VERSION : " << version << std::endl;
        }
    }

    return version;
}

bool Manifest::InsertVersionNumber(unsigned int version) const
{
    char ver[256];
    snprintf(ver, 256, "%u", version);

    char pexc[1024];
    snprintf( pexc,
              1024, 
              "INSERT OR REPLACE INTO \"%s\" (key, value) VALUES (\"%s\", \"%s\");",
              g_metatable.c_str(),
              "version",
              ver
            ); 

    return PerformQuery(pexc);
}

bool Manifest::InsertPostID(const std::string &postID) const
{
    char pexc[1024];
    snprintf( pexc,
              1024, 
              "INSERT OR REPLACE INTO \"%s\" (key, value) VALUES (\"%s\", \"%s\");",
              g_metatable.c_str(),
              "post",
              postID.c_str() 
            ); 

    return PerformQuery(pexc);
}

bool Manifest::QueryForFile(const std::string &filename, FileInfo* out)
{
    char pexc[1024];

    snprintf( pexc,
              1024, 
              "SELECT * FROM %s WHERE filename=\"%s\";",
              g_infotable.c_str(),
              filename.c_str()
            );
     
    std::cout<< " EXECING : " << pexc << std::endl;

    SelectResult res;
    if(!PerformSelect(pexc, res))
        return false;

    std::cout << " Row count : " << res.nRow << std::endl;
    std::cout << " Col count : " << res.nCol << std::endl;

    int step = 0;
    for(int i=0; i<res.nRow+1; i++)
    {
        step = i*res.nCol;
        std::cout<< " step : " << step << std::endl;

        for(int j=0; j<res.nCol; j++)
        {
            std::cout << " Results : " << res.results[j+step] << std::endl;
        }

        if(step > 0)
        {
            out->SetFilename(res.results[0+step]);
            out->SetFilepath(res.results[1+step]);
            out->SetChunkName(res.results[2+step]);
            out->SetChunkCount(res.results[3+step]);
            out->LoadSerializedChunkData(res.results[4+step]);
            out->SetFileSize(res.results[5+step]);
            out->SetPostID(res.results[6+step]);
            out->SetChunkPostID(res.results[7+step]);
            out->SetPostVersion(res.results[8+step]);
            out->SetKey(res.results[9+step]);
        }
    }

    sqlite3_free_table(res.results);

    return true;
}

//"CREATE TABLE IF NOT EXISTS %s (filename TEXT, filepath TEXT, chunkname TEXT, chunkcount INT, chunkdata BLOB, filesize INT, metapostid TEXT, chunkpostid TEXT, postversion INT, key BLOB, PRIMARY KEY(filename ASC));",
              
bool Manifest::InsertFileInfoToDb(const FileInfo* fi)
{
    if(!m_pDb)
    {
        std::cout<<"fail db " <<std::endl;
        return false;
    }
    if(!fi)
        return false;

    std::string filename, filepath, chunkname, chunkdata, metapostid, chunkpostid, key;

    fi->GetFilename(filename);
    fi->GetFilepath(filepath);
    fi->GetChunkName(chunkname);
    fi->GetSerializedChunkData(chunkdata);
    fi->GetPostID(metapostid);
    fi->GetChunkPostID(chunkpostid);

    Credentials cred = fi->GetCredentialsCopy();
    cred.GetKey(key);

    std::cout<< " name : " << filename << std::endl;
    std::cout<< " path : " << filepath << std::endl;
    std::cout<< " chunk name : " << chunkname << std::endl;
    std::cout<< " count : " << fi->GetChunkCount() << std::endl;
    std::cout<< " filesize : " << fi->GetFileSize() << std::endl;
    std::cout<< " meta id : " << metapostid << std::endl;
    std::cout<< " chunk id : " << chunkpostid << std::endl;
    std::cout<< " version : " << fi->GetPostVersion() << std::endl;
    std::cout<< " chunkdata : " << chunkdata << std::endl;
    std::cout<< " key : " << key << std::endl;

    std::string query;
    query += "INSERT OR REPLACE INTO ";
    query += g_infotable;
    query += " (filename, filepath, chunkname, chunkcount, chunkdata, filesize, metapostid, chunkpostid, postversion, key) VALUES (?,?,?,?,?,?,?,?,?,?);";


    // Prepare statement
    sqlite3_stmt* stmt = NULL;
    int ret = sqlite3_prepare_v2(m_pDb, query.c_str(), -1, &stmt, 0);

    if(ret == SQLITE_OK)
    {
        if(stmt)
        {
            ret = sqlite3_bind_text(stmt, 1, filename.c_str(), filename.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 2, filepath.c_str(), filepath.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 3, chunkname.c_str(), chunkname.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 4, fi->GetChunkCount());
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_blob(stmt, 5, chunkdata.c_str(), chunkdata.size(), SQLITE_TRANSIENT);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 6, fi->GetFileSize());
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 7, metapostid.c_str(), metapostid.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 8, chunkpostid.c_str(), chunkpostid.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_int(stmt, 9, fi->GetPostVersion());
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_blob(stmt, 10, key.c_str(), key.size(), SQLITE_TRANSIENT);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_step(stmt);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_finalize(stmt);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }
        }
        else
        {
            std::cout<< "Invalid statement" << std::endl;
            printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
            return false;
        }

    }
    else
    {
        std::cout<< "failed to prepare statement " << std::endl;
        printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
        return false;
    }

    return true;
}


bool Manifest::InsertFilePostID(const std::string& filename, const std::string &id)
{
    char pexc[1024];

    snprintf( pexc,
              1024, 
              "UPDATE \"%s\" SET metapostid=\"%s\" WHERE filename=\"%s\";",
              g_infotable.c_str(),
              id.c_str(),
              filename.c_str()
            ); 

    std::cout << "INSERT POST ID : " << pexc << std::endl;

    return PerformQuery(pexc);
}

bool Manifest::InsertFileChunkPostID(const std::string &filename, const std::string &id)
{
    char pexc[1024];

    snprintf( pexc,
              1024, 
              "UPDATE \"%s\" SET chunkpostid=\"%s\" WHERE filename=\"%s\";",
              g_infotable.c_str(),
              id.c_str(),
              filename.c_str()
            ); 

    std::cout << "INSERT CHUNK POST ID : " << pexc << std::endl;

    return PerformQuery(pexc);
}

//" (name TEXT, path TEXT, children TEXT, postid TEXT, PRIMARY KEY(path ASC));";
// FolderTable
bool Manifest::InsertFolderData( const std::string &name, 
                                 const std::string &path,
                                 const std::string &children,
                                 const std::string &postid)
{
    std::string query;
    query += "INSERT OR REPLACE INTO ";
    query += g_foldertable;
    query += " (name, path, children, postid) VALUES (?,?,?,?);";

    // Prepare statement
    sqlite3_stmt* stmt = NULL;
    int ret = sqlite3_prepare_v2(m_pDb, query.c_str(), -1, &stmt, 0);

    if(ret == SQLITE_OK)
    {
        if(stmt)
        {
            ret = sqlite3_bind_text(stmt, 1, name.c_str(), name.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 2, path.c_str(), path.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 3, children.c_str(), children.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_bind_text(stmt, 4, postid.c_str(), postid.size(), SQLITE_STATIC);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_step(stmt);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }

            ret = sqlite3_finalize(stmt);
            if(ret != SQLITE_OK)
            {
                printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
                return false;
            }
        }
        else
        {
            std::cout<< "Invalid statement" << std::endl;
            printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
            return false;
        }
    }
    else
    {
        std::cout<< "failed to prepare statement " << std::endl;
        printf("Error message: %s\n", sqlite3_errmsg(m_pDb));
        return false;
    }

    return true;
}

bool Manifest::RemoveFolderData(const std::string& folderpath)
{

    return false;
}

bool Manifest::QueryForFolderData( const std::string& folderpath,
                                   std::string &nameOut,
                                   std::string &pathOut,
                                   std::string &childrenOut,
                                   std::string &postidOut)
{
    std::string query;
    query += "SELECT * FROM ";
    query += g_foldertable;
    query += " WHERE filename=\"";
    query += folderpath;
    query += "\";";

    SelectResult res;
    if(!PerformSelect(query.c_str(), res))
        return false;

    std::cout << " Row count : " << res.nRow << std::endl;
    std::cout << " Col count : " << res.nCol << std::endl;

    int step = 0;
    for(int i=0; i<res.nRow+1; i++)
    {
        step = i*res.nCol;
        std::cout<< " step : " << step << std::endl;

        for(int j=0; j<res.nCol; j++)
        {
            std::cout << " Results : " << res.results[j+step] << std::endl;
        }

        if(step > 0)
        {
            nameOut = res.results[0+step];
            pathOut = res.results[1+step];
            childrenOut = res.results[2+step];
            postidOut = res.results[3+step];
        }
    }

    sqlite3_free_table(res.results);
    return false;
}

bool Manifest::InsertCredentialsToDb(const FileInfo* fi)
{
    // TODO :: This
    if(!fi)
        return false;

    return false; // should be true
}

bool Manifest::RemoveFileFromDb(const std::string &filename)
{
    // TODO :: this
    std::cout<<" HERE " << std::endl;
    char pexc[1024];

    snprintf( pexc,
              1024, 
              "DELETE FROM \"%s\"  WHERE filename=\"%s\";",
              g_infotable.c_str(),
              filename.c_str()
            ); 

    std::cout << "DELETE : " << pexc << std::endl;
    return PerformQuery(pexc);
}

bool Manifest::InsertFileInfo(FileInfo* fi)
{
    if(!fi)
        return false;
    
    //TODO :: expand checks for valid file etc
    //        for now just insert and be done.
    //TODO :: select an in memory strategy for files recently 
    //        accessed. Should we keep all files (file info objects
    //        in memory upon use? Should we load them in?, should
    //        we set perhaps a half life?

    bool status = InsertFileInfoToDb(fi);
    if(status)
    {
        SetIsDirty(true);
    }

    return status;
}

bool Manifest::RemoveFileInfo(const std::string &filename)
{
    bool status = RemoveFileFromDb(filename);
    if(status)
    {
        SetIsDirty(true);
    }

    return status;
}


bool Manifest::IsFileInManifest(const std::string &filename)
{
    return QueryForFileExistence(filename);
}

void Manifest::CompareAndMergeDb(sqlite3* pDb)
{
    // Select * db

}

