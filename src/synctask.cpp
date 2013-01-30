#include "synctask.h"

#include "atticpost.h"                  
#include "urlparams.h"                  
#include "constants.h"                  
#include "errorcodes.h"                 
#include "conoperations.h"
#include "postutils.h"
#include "utils.h"                      

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
              TentTask( pApp,                                 
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

void SyncTask::RunTask()
{
    int status = ret::A_OK;

    // Sync meta data posts and populate the manifest
    status = SyncMetaData(); 
    
    if(status == ret::A_OK)
    {
        // Run a pull all files afterwards
    }

    Callback(status, NULL);
}

int SyncTask::SyncMetaData()
{
    int status = ret::A_OK;
    
    int postcount = GetAtticPostCount();
                                                                                                  
    if(postcount > 0)
    {
        std::string url;                                                                   
        GetEntityUrl(url);                                                                    
        url += "/tent/posts";  // TODO :: make provider agnostic

        std::cout<<" URL : " << url << std::endl;

        UrlParams params;                                                                  
        params.AddValue(std::string("post_types"), std::string(cnst::g_szFileMetadataPostType));  
        params.AddValue(std::string("limit"), std::string("200"));                         

        Response response;                                                                 
        AccessToken* at = GetAccessToken();                                                
        conops::HttpGet( url,
                         &params,
                         *at,
                         response); 

        std::cout<< " CODE : " << response.code << std::endl;                              
        std::cout<< " RESPONSE : " << response.body << std::endl;                          

        if(response.code == 200)
        {
            // Parse Response
            Json::Value root;                               
            Json::Reader reader;                            
                                                               
            if(reader.parse(response.body, root))          
            {
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
                        std::vector<std::string>* pChunkPosts;
                        pChunkPosts = p.GetChunkPosts();

                        if(pChunkPosts)
                        {
                            std::vector<std::string>::iterator itr = pChunkPosts->begin();

                            for(;itr != pChunkPosts->end(); itr++)
                            {
                                std::cout<<" chunk post : " << *itr << std::endl;
                                // Pull chunk post
                                std::string url;
                                GetEntityUrl(url);
                                url += "/tent/posts";  // TODO :: make provider agnostic


                            }

                        }
  
  

                    }
            }
            else
            {
                status = ret::A_FAIL_JSON_PARSE;              
            }
        }
        else
        {
            status = ret::A_FAIL_NON_200;
        }

    }
    else if(postcount == -1)
    {
        status = ret::A_FAIL_NON_200;
    }
    
                   
    return status;
}

int SyncTask::GetAtticPostCount()                                                            
{                                                                                                 
    std::string url;
    GetEntityUrl(url);

    // TODO :: make this provider agnostic                                                         
    url += "/tent/posts/count";

    std::cout<<" URL : " << url << std::endl;

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(cnst::g_szAtticPostType));             

    Response response;                                                                            
    AccessToken* at = GetAccessToken();                                                           
    conops::HttpGet( url,
                     &params,
                     *at,
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
