#include "polltask.h"

#include <string>
#include <iostream>
#include "netlib.h"


static const int g_instance_count = 0;

PollTask::PollTask( TentApp* pApp,
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
                    TentTask( Task::POLL,
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

PollTask::~PollTask() 
{
}

void PollTask::OnStart()
{
}

void PollTask::OnPaused()
{
}

void PollTask::OnFinished()
{
}

void PollTask::RunTask()
{
    // Spin off consumer task for checking each file meta post for newer versions
    if(!g_instance_count) {
        g_instance_count++;
        // Poll for folder posts
        // Update Entries on a counter
        // Update pull

        g_instance_count = 0;
    }
    else {
        status = ret::A_FAIL_RUNNING_SINGLE_INSTANCE;
        Callback(status, NULL);
        SetFinishedState();
    }

}

int PollTask::SyncFolderPosts()
{
    int status = ret::A_OK;
    // Get Folder Posts
    int postcount = GetAtticPostCount();
    if(postcount > 0) {
        Entity entity;
        GetEntity(entity);
        std::string url;                                                                   
        entity.GetApiRoot(url);
        utils::CheckUrlAndAppendTrailingSlash(url);
        url += "posts";

        AccessToken* at = GetAccessToken();                                                           
        int cap = 200;
        std::string lastid;
        for(int i=0; i < postcount; i+=cap) {
            int diff = postcount - i;
            int request_count = 0;
            if(diff > cap)
                request_count = cap;
            else
                request_count = diff;

            char countBuff[256] = {"\0"};
            snprintf(countBuff, 256, "%d", request_count);

            UrlParams params;                                                                  
            params.AddValue(std::string("post_types"), std::string(cnst::g_szFolderPostType));  
            params.AddValue(std::string("limit"), std::string(countBuff));                         
            if(!lastid.emtpy()) 
                params.AddValue(std::string("last_id"), lastid);

            Response resp;
            netlib::HttpGet( url,
                             &params,
                             at,
                             resp);
            std::cout<<" response code : " << resp.code << std::endl;
            std::cout<<" response body : " << resp.body << std::endl;

            if(resp.code == 200) { 
                // Loop through all the responses


            }
            else {
                break;
            }

        }
    }
    return status;
}

int PollTask::GetAtticPostCount()
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
