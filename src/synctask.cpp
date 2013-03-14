#include "synctask.h"

#include <vector>

#include "pullalltask.h"
#include "atticpost.h"                  
#include "urlparams.h"                  
#include "constants.h"                  
#include "errorcodes.h"                 
#include "postutils.h"
#include "utils.h"                      
#include "netlib.h"

static SyncTask* g_pCurrent = NULL;


SyncTask::SyncTask( TentApp* pApp,
              FileManager* pFm,
              CredentialsManager* pCm,
              TaskArbiter* pTa,
              TaskFactory* pTf,
              const AccessToken& at,
              const Entity& entity,
              const std::string& filepath,
              const std::string& tempdir,
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*))
              :                                               
              TentTask( Task::SYNC,
                        pApp,                                 
                        pFm,                                  
                        pCm,                                  
                        pTa,                                  
                        pTf,                                  
                        at,                                   
                        entity,                               
                        filepath,                             
                        tempdir,                              
                        workingdir,                           
                        configdir,                            
                        callback)                             
{
}

SyncTask::~SyncTask()
{
}

void SyncTask::OnStart()
{ 

} 

void SyncTask::OnPaused()
{ 

} 

void SyncTask::OnFinished() 
{ 
}

void SyncTask::RunTask()
{
    int status = ret::A_OK;

    if(!g_pCurrent) { 
        g_pCurrent = this;
        // Sync meta data posts from the manifest, obtained from the folder table,
        // check if they exist in the infotable
        //      if not insert it
        // check if file exists
        //      if not spin off a pull task

        status = SyncMetaData(); 
        Callback(status, NULL);
        SetFinishedState();
    }
    else {
        status = ret::A_FAIL_RUNNING_SINGLE_INSTANCE;
        Callback(status, NULL);
        SetFinishedState();
    }
}

int SyncTask::SyncMetaData()
{
    int status = ret::A_OK;
    
    int postcount = GetAtticPostCount();
                                                                                                  
    if(postcount > 0) { 
        Entity entity;
        GetEntity(entity);
             
        std::string url;                                                                   
        entity.GetApiRoot(url);
        utils::CheckUrlAndAppendTrailingSlash(url);
        url += "posts";

        std::cout<<" URL : " << url << std::endl;

        UrlParams params;                                                                  
        params.AddValue(std::string("post_types"), std::string(cnst::g_szFileMetadataPostType));  
        params.AddValue(std::string("limit"), std::string("200"));                         

        Response response;                                                                 
        AccessToken* at = GetAccessToken();                                                
        netlib::HttpGet( url,
                         &params,
                         at,
                         response); 

        std::cout<< " CODE : " << response.code << std::endl;
        //std::cout<< " RESPONSE : " << response.body << std::endl;                          

        if(response.code == 200)
        {
            // Parse Response
            Json::Value root;                               
            Json::Reader reader;                            
                                                              
            std::cout << " parsing response body " << std::endl;
            if(reader.parse(response.body, root)) {  
                std::vector<FileInfo> fileInfoList;

                std::cout<<" root size : " << root.size() << std::endl;

                Json::ValueIterator itr = root.begin();         
                int count = 0;                                  
                for(;itr != root.end(); itr++)                  
                {                                               
                    AtticPost p;
                    p.Deserialize(*itr);                                             
                    count++;

                    std::string name;
                    p.GetAtticPostFilename(name);
                    std::cout<< " Filename : " << name << std::endl;

                    FileInfo fi;
                    postutils::DeserializeAtticPostIntoFileInfo(p, fi);
                    std::string path;
                    fi.GetFilepath(path);
                    std::cout<< " filepath : " << path << std::endl;

                    // Get Chunk info
                    std::vector<std::string> chunkPosts;
                    chunkPosts = p.GetChunkPosts();

                    if(chunkPosts.size())
                    {
                        std::cout<<" chunk post : " << *itr << std::endl;
                        std::cout<<" number of chunk posts : " << chunkPosts.size() << std::endl;
                        std::cout<<" chunk post : " << chunkPosts[0] << std::endl;

                        std::string chunkposturl;
                        entity.GetApiRoot(chunkposturl);
                        chunkposturl += "/posts/";

                        std::vector<std::string>::iterator itr = chunkPosts.begin();
                        std::string postid;
                        for(;itr != chunkPosts.end(); itr++)
                        {
                            fi.SetChunkPostID(*itr);
                            postid.clear();
                            postid = *itr;
                            chunkposturl += postid;

                            response.clear();
                            netlib::HttpGet( chunkposturl, 
                                             &params,
                                             at,
                                             response); 

                            std::cout<< " CODE : " << response.code << std::endl;
                            //std::cout<< " RESP : " << response.body << std::endl;

                            if(response.code == 200) {
                                ChunkPost cp;
                                jsn::DeserializeObject(&cp, response.body);
                                if(cp.GetChunkSize()) {
                                    std::cout<<" THIS ChunkPost : " << cp.GetChunkSize() << std::endl;
                                    std::vector<ChunkInfo>* ciList = cp.GetChunkList();
                                    std::vector<ChunkInfo>::iterator itr = ciList->begin();

                                    for(;itr != ciList->end(); itr++) {
                                        fi.PushChunkBack(*itr);
                                    }

                                }

                                std::cout<<" CHUNK COUNT : " << fi.GetChunkCount() << std::endl;
                                fileInfoList.push_back(fi);
                                InsertFileInfoToManager(fileInfoList);
                            }
                        }
                    }
                    else {
                        std::cout<<" no chunks ... : " << chunkPosts.size() << std::endl;
                    }


                }
            }
            else {
                status = ret::A_FAIL_JSON_PARSE;              
            }
        }
        else {
            status = ret::A_FAIL_NON_200;
        }

    }
    else if(postcount == -1)
    {
        status = ret::A_FAIL_NON_200;
    }
    
                   
    return status;
}

int SyncTask::InsertFileInfoToManager(const std::vector<FileInfo>& filist)
{
    std::cout<<" Inserting .... " << std::endl;
    int status = ret::A_OK;
    FileManager* fm = GetFileManager();

    std::vector<FileInfo>::const_iterator itr = filist.begin();

    for(;itr != filist.end(); itr++) {
        // Need to resolve local paths
        FileInfo fi = *itr;
        fm->InsertToManifest(&fi);
    }

    return status;
}

int SyncTask::GetAtticPostCount()                                                            
{                                                                                                 
    std::string url;
    GetEntityUrl(url);
    utils::CheckUrlAndAppendTrailingSlash(url);
    url += "posts/count";

    std::cout<<" URL : " << url << std::endl;

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(cnst::g_szAtticPostType));             

    Response response;                                                                            
    AccessToken* at = GetAccessToken();                                                           
    netlib::HttpGet( url,
                     &params,
                     at,
                     response);

    std::cout<< "CODE : " << response.code << std::endl;                                          
    std::cout<< "RESPONSE : " << response.body << std::endl;                                      

    int count = -1;                                                                               
    if(response.code == 200) {
        count = atoi(response.body.c_str());
    }

    return count;
}    



