#include "polltask.h"

#include <string>
#include <iostream>

#include "netlib.h"
#include "constants.h"
#include "folderpost.h"
#include "eventsystem.h"

namespace polltask 
{
    static PollTask* g_pCurrentPollTask = NULL;

    static void PollTaskCB(int a, void* b)
    {
        if(g_pCurrentPollTask)
            g_pCurrentPollTask->PollTaskCB(a, b);
    }
}

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

void PollTask::PollTaskCB(int a, void* b)
{
    std::cout<<" POLL TASK CALLBACK HIT " << std::endl;

}

void PollTask::RunTask()
{
    int status = ret::A_OK;
    // Spin off consumer task for checking each file meta post for newer versions
    if(!polltask::g_pCurrentPollTask) {
        polltask::g_pCurrentPollTask = this;
        // Poll for folder posts
        // Update Entries on a counter
        // Update pull
        status = SyncFolderPosts();

        Callback(status, NULL);
        SetFinishedState();
        polltask::g_pCurrentPollTask = NULL;
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
    int postcount = GetFolderPostCount();
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
            if(!lastid.empty())
                params.AddValue(std::string("last_id"), lastid);

            Response resp;
            netlib::HttpGet( url,
                             &params,
                             at,
                             resp);

            std::cout<< "LINK HEADER : " << resp.header["Link"] << std::endl;
            std::cout<<" response code : " << resp.code << std::endl;
            std::cout<<" response body : " << resp.body << std::endl;

            if(resp.code == 200) { 
                // Loop through all the responses
                // For each folder post spin off a sync task for now, just do it serially
                // TODO :: make sure to spin off a task after new threading and message passing 
                //         framework is in place.

                // Parse json
                std::deque<Folder> folders;
                Json::Value root;
                Json::Reader reader;
                reader.parse(resp.body, root);

                jsn::PrintOutJsonValue(&root);

                std::cout<<" entries : " << root.size() << std::endl;
                // extract since id
                Json::ValueIterator itr = root.begin();
                for(; itr != root.end(); itr++) {
                    FolderPost fp;
                    jsn::DeserializeObject(&fp, *itr);
                    Folder folder;
                    fp.GetFolder(folder);
                    folders.push_back(folder);
                    SyncFolder(folder);
                }
            }
            else {
                break;
            }
        }
    }
    return status;
}

int PollTask::SyncFolder(Folder& folder)
{
    // Make sure the folder exists in the manifest
    //
    // loop through the entries make sure they exist, if there is a newer version
    // spin off a pull command
    std::cout<<" Syncing ... folder ... " << std::endl;
    int status = ret::A_OK;
    Folder::EntryList* pList = folder.GetEntryList();

    if(pList) { 
        std::cout<<" ENTRY SIZE : " << pList->size() << std::endl;
        Folder::EntryList::iterator itr = pList->begin();
        for(;itr != pList->end(); itr++) {
            std::string postid;
            itr->second.GetPostID(postid);
            std::cout<<" FOLDER ENTRY POST ID : " << postid << std::endl;
            if(!postid.empty()) { 
                // Check if currently in the sync queue
                    // if no sync
                    // if yes ignore
                Event event;
                event.type = Event::REQUEST_SYNC_POST;
                event.value = postid;
                event.callback = polltask::PollTaskCB;
                evnt::RaiseEvent(event);
            }


        }
    }
    else {
        std::cout<<" invalid entry list " << std::endl;
    }

    return status;
}

int PollTask::GetFolderPostCount()
{                                                                                                 
    std::string url;
    GetEntityUrl(url);
    utils::CheckUrlAndAppendTrailingSlash(url);
    url += "posts/count";

    std::cout<<" URL : " << url << std::endl;

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(cnst::g_szFolderPostType));             

    Response response;                                                                            
    AccessToken* at = GetAccessToken();                                                           
    netlib::HttpGet( url,
                     &params,
                     at,
                     response);

    int count = -1;                                                                               
    if(response.code == 200) {
        count = atoi(response.body.c_str());
    }

    return count;
}
