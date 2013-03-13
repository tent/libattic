#include "deleteallpoststask.h"

#include <vector>
#include <string>

#include "atticpost.h"                  
#include "urlparams.h"    
#include "constants.h"
#include "errorcodes.h"
#include "postutils.h"
#include "utils.h"
#include "netlib.h"



DeleteAllPostsTask::DeleteAllPostsTask( TentApp* pApp, 
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
                                        TentTask( Task::DELETEALLPOSTS,
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
                                                  callback )
{

}

DeleteAllPostsTask::~DeleteAllPostsTask()
{

}

void DeleteAllPostsTask::OnStart() { } 
void DeleteAllPostsTask::OnPaused() { } 
void DeleteAllPostsTask::OnFinished() { }

void DeleteAllPostsTask::RunTask()
{
    int status = ret::A_OK;

    Entity entity;
    GetEntity(entity);
         
    std::string url;                                                                   
    entity.GetApiRoot(url);
    url += "/posts";

    UrlParams params;                                                                  
    params.AddValue(std::string("post_types"), std::string(cnst::g_szFileMetadataPostType));  
    params.AddValue(std::string("limit"), std::string("200"));                         

    Response response;                                                                 
    AccessToken* at = GetAccessToken();                                                
    netlib::HttpGet( url,
                     &params,
                     at,
                     response); 

    if(response.code == 200)
    {
        // Parse Response
        Json::Value root;                               
        Json::Reader reader;                            

        Entity entity;
        GetEntity(entity);

        std::vector<std::string> postList;
                                                           
        if(reader.parse(response.body, root))          
        {  
            std::vector<FileInfo> fileInfoList;

            Json::ValueIterator itr = root.begin();         
            int count = 0;                                  
            for(;itr != root.end(); itr++)                  
            {                                               
                AtticPost p;
                p.Deserialize(*itr);     

                // Get Post id
                std::string fpostid;
                p.GetID(fpostid);
                postList.push_back(fpostid);

                // Get Chunk info
                std::vector<std::string> chunkPosts;
                chunkPosts = *p.GetChunkPosts();

                if(chunkPosts.size())
                {
                    std::string chunkposturl;
                    entity.GetApiRoot(chunkposturl);
                    chunkposturl += "/posts/";

                    std::vector<std::string>::iterator itr = chunkPosts.begin();
                    for(;itr != chunkPosts.end(); itr++)
                    {
                        postList.push_back(*itr);
                    }
                }
            }
        }

        std::cout<<" NUMBER OF POST ID's : " << postList.size() << std::endl;
        std::vector<std::string>::iterator itr = postList.begin();
        for(; itr != postList.end(); itr++)
        {
            std::cout<< *itr << std::endl;
            DeletePost(*itr);
        }
    }

    Callback(status, NULL);
    SetFinishedState();
}

int DeleteAllPostsTask::DeletePost(const std::string& postId)
{
    // Modify Post
    Entity entity;
    GetEntity(entity);

    std::string posturl; 
    entity.GetApiRoot(posturl);
    posturl += "/posts/";
    posturl += postId;

    std::cout<< " DELETE URL : " << posturl << std::endl;
    AccessToken* at = GetAccessToken();

    Response response;
    netlib::HttpDelete( posturl,
                        NULL,
                        at,
                        response);

    std::cout<<"Code : " << response.code << std::endl;
    std::cout<<"RESPONSE : " << response.body << std::endl;

    return ret::A_OK;
}   

