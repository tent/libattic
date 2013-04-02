#include "polltask.h"

#include <iostream>

#include "netlib.h"
#include "constants.h"
#include "folderpost.h"
#include "event.h"
#include "taskdelegate.h"
#include "sleep.h"

namespace attic {

namespace polltask {
    static PollTask* g_pCurrentPollTask = NULL;

    static void PollTaskCB(int a, void* b)
    {
    }
}

static long total_elapsed = 0;
static boost::timer::nanosecond_type const limit(10 * 1000000000LL); // 10 seconds in nanoseconds

PollTask::PollTask( FileManager* pFm,
                    CredentialsManager* pCm,
                    const AccessToken& at,
                    const Entity& entity,
                    const std::string& filepath,
                    const std::string& tempdir,
                    const std::string& workingdir,
                    const std::string& configdir,
                    TaskDelegate* callbackDelegate)
                    :                                               
                    TentTask( Task::POLL,
                              pFm,
                              pCm,
                              at,
                              entity,                               
                              filepath,                             
                              tempdir,                              
                              workingdir,                           
                              configdir,                            
                              callbackDelegate)                             
{
    m_pDelegate = new PollDelegate(this);
    running_ = true;
}

PollTask::~PollTask() {
    if(m_pDelegate) {
        delete m_pDelegate;
        m_pDelegate = NULL;
    }
}

void PollTask::OnStart(){
    if(!polltask::g_pCurrentPollTask) {
        polltask::g_pCurrentPollTask = this;
    }

    event::RegisterForEvent(this, event::Event::PAUSE);
    event::RegisterForEvent(this, event::Event::RESUME);
    timer_.start();
}

void PollTask::OnPaused() {}

void PollTask::OnFinished() {
    event::UnregisterFromEvent(this, event::Event::PAUSE);
    event::UnregisterFromEvent(this, event::Event::RESUME);
    timer_.stop();
}

void PollTask::OnEventRaised(const event::Event& event){
    switch(event.type){
        case event::Event::PAUSE:
            running_ = false;
            break;
        case event::Event::RESUME:
            running_ = true;
            break;
        default:
            std::cout<<" Unknown event : " << event.type << std::endl;
    };
}
void PollTask::PollTaskCB(int a, std::string& b) {
    //std::cout<<" POLL TASK CALLBACK HIT " << std::endl;
    std::string returnpost = b;
    if(m_ProcessingQueue.find(returnpost) != m_ProcessingQueue.end()) {
        // remove it from the map
        m_ProcessingQueue.erase(returnpost);
    }
    else {
        std::cout<<" POSTID NOT FOUND IN QUEUE ?!?!? " << b << std::endl;
    }
}

// TODO:: after v0.1 abstract this to sync strategy
void PollTask::RunTask() {
    int status = ret::A_OK;

    // Spin off consumer task for checking each file meta post for newer versions
    if(polltask::g_pCurrentPollTask == this) {
        if(!timer_.is_stopped()) {
            boost::timer::cpu_times time = timer_.elapsed();
            boost::timer::nanosecond_type const elapsed(time.system + time.user);
            total_elapsed += elapsed;
        }

        //std::cout<<" total elapsed : " << total_elapsed << "limit " << limit << std::endl;
        if(total_elapsed > limit) {
            std::cout<<" ********************************************************" << std::endl;
            std::cout<<" POLLING - ELAPSED : " << total_elapsed << std::endl;
            std::cout<<" ********************************************************" << std::endl;
            total_elapsed = 0;
            timer_.stop();
            if(running_) {
                status = SyncFolderPosts();
                if(status != ret::A_OK)
                    std::cout<<" POLLING ERR : " << status << std::endl;
            }
            timer_.start();
        }
    }
    else {
        std::cout<<" FAIL MULTI INSTANCE " << std::endl;
        status = ret::A_FAIL_RUNNING_SINGLE_INSTANCE;
        Callback(status, NULL);
        SetFinishedState();
    }

    sleep::sleep_milliseconds(100);
    //sleep::sleep_seconds(2);
}

int PollTask::SyncFolderPosts() {
    int status = ret::A_OK;
    // Get Folder Posts
    int postcount = GetFolderPostCount();
    if(postcount > 0) {
        Entity entity = TentTask::entity();
        std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();

        AccessToken at = access_token();
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
            params.AddValue(std::string("post_types"), std::string(cnst::g_attic_folder_type));  
            params.AddValue(std::string("limit"), std::string(countBuff));                         
            if(!lastid.empty())
                params.AddValue(std::string("last_id"), lastid);

            Response resp;
            netlib::HttpGet(posts_feed,
                            &params,
                            &at,
                            resp);

          //  std::cout<< "LINK HEADER : " << resp.header["Link"] << std::endl;
            //std::cout<<" response code : " << resp.code << std::endl;
            //std::cout<<" response body : " << resp.body << std::endl;

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

           //     std::cout<<" entries : " << root.size() << std::endl;
                // extract since id
                Json::ValueIterator itr = root.begin();
                for(; itr != root.end(); itr++) {
                    FolderPost fp;
                    jsn::DeserializeObject(&fp, *itr);
                    Folder folder = fp.folder();
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

int PollTask::SyncFolder(Folder& folder) {
    // Make sure the folder exists in the manifest
    //
    // loop through the entries make sure they exist, if there is a newer version
    // spin off a pull command
    //std::cout<<" Syncing ... folder ... " << std::endl;
    int status = ret::A_OK;
    Folder::EntryList* pList = folder.GetEntryList();

    if(pList) { 
        //std::cout<<" ENTRY SIZE : " << pList->size() << std::endl;
        Folder::EntryList::iterator itr = pList->begin();
        for(;itr != pList->end(); itr++) {
            std::string postid;
            itr->second.GetPostID(postid);
            //std::cout<<" FOLDER ENTRY POST ID : " << postid << std::endl;
            if(!postid.empty()) { 
                // Check if currently in the sync queue
                if(m_ProcessingQueue.find(postid) == m_ProcessingQueue.end()) {
                    m_ProcessingQueue[postid] = true;
                    // TODO :: create TaskDelegate to pass here !
                    event::RaiseEvent(event::Event::REQUEST_SYNC_POST, postid, m_pDelegate);
                }
                // If it is in the queue ignore, do not reaise an event
            }
        }
    }
    else {
        std::cout<<" invalid entry list " << std::endl;
    }

    return status;
}

int PollTask::GetFolderPostCount() {
    std::string url = entity().GetPreferredServer().posts_feed();
    //std::cout<<" URL : " << url << std::endl;

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(cnst::g_attic_folder_type));             

    Response response;                                                                            
    AccessToken at = access_token();                                                           
    netlib::HttpHead(url,
                    &params,
                    &at,
                    response);

    //std::cout<<" code : " << response.code << std::endl;
    //std::cout<<" header : " << response.header.asString() << std::endl;
    //std::cout<<" body : " << response.body << std::endl;

    int count = -1;                                                                               
    if(response.code == 200) {
        if(response.header.HasValue("Count"))
            count = atoi(response.header["Count"].c_str());
    }

    return count;
}

void PollTask::PushBackFile(const std::string& filepath) {
    m_ProcessingQueue[filepath] = true;
}

void PollTask::RemoveFile(const std::string& filepath) {
    m_ProcessingQueue.erase(filepath);
}

bool PollTask::IsFileInQueue(const std::string& filepath) {
    if(m_ProcessingQueue.find(filepath)!= m_ProcessingQueue.end())
        return true;
    return false;
}


}//namespace
