#include "synctask.h"

#include <vector>

#include "pullalltask.h"
#include "atticpost.h"                  
#include "urlparams.h"                  
#include "constants.h"                  
#include "errorcodes.h"                 
#include "conoperations.h"
#include "postutils.h"
#include "utils.h"                      
#include "netlib.h"

static SyncTask* g_pCurrent = NULL;

void SyncCallBack(int a, void* b)
{
    if(g_pCurrent)
    {
        g_pCurrent->SyncCb(a,b);
    }
}

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
    int status = ret::A_OK;

    g_pCurrent = NULL;

    if(m_CallbackCount > m_CallbackHit)
        status = ret::A_FAIL_TIMED_OUT;

    Callback(status, NULL);
}

void SyncTask::RunTask()
{
    int status = ret::A_OK;

    if(!g_pCurrent)
    {
        g_pCurrent = this;


        // Sync meta data posts and populate the manifest
        status = SyncMetaData(); 
        
        if(status == ret::A_OK)
        {
            std::cout<<" Spinning off pull task " << std::endl;
            // Run a pull all files afterwards
            status = SpinOffPullAllTask();
            /*
            if(status == ret::A_OK)
            {
                //wait
                for(;;)
                {
                    if(m_CallbackCount <= m_CallbackHit)
                    {
                        break;
                    }
                    sleep(0);
                }
            }
            */
        }
    }
    else
    {
        //status = ret::A_FAIL_RUNNING_SINGLE_INSTANCE;
        //Callback(status, NULL);
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
                    chunkPosts = *p.GetChunkPosts();

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
                                JsonSerializer::DeserializeObject(&cp, response.body);
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
        fm->InsertToManifest(&*itr);
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
    if(response.code == 200)
    {
        count = atoi(response.body.c_str());                                                          
    }

    return count;
}    

int SyncTask::SpinOffPullAllTask()
{
    int status = ret::A_OK;
    TaskFactory* tf = GetTaskFactory();
    TaskArbiter* ta = GetTaskArbiter();
    std::string entityurl, tempdir, workingdir, configdir;
    GetEntityUrl(entityurl);
    GetTempDirectory(tempdir);
    GetWorkingDirectory(workingdir);
    GetConfigDirectory(configdir);

    Entity entity;
    GetEntity(entity);

    Task* t = tf->GetTentTask( Task::PULLALL,
                               GetTentApp(), 
                               GetFileManager(), 
                               GetCredentialsManager(),
                               GetTaskArbiter(),
                               GetTaskFactory(),
                               GetAccessTokenCopy(),
                               entity,
                               "",               
                               tempdir,          
                               workingdir,       
                               configdir,        
                               &SyncCallBack);      

    
    ta->SpinOffTask(t);
    m_CallbackCount++;

    return status;
}

void SyncTask::SyncCb(int a, void* b)
{
    m_CallbackHit++;
    if(m_CallbackCount <= m_CallbackHit)
    {
        SetFinishedState();
    }
}

