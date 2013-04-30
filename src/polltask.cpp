#include "polltask.h"

#include <iostream>

#include "netlib.h"
#include "constants.h"
#include "folderpost.h"
#include "event.h"
#include "taskdelegate.h"
#include "sleep.h"
#include "logutils.h"

namespace attic {

namespace polltask {
    static PollTask* g_pCurrentPollTask = NULL;

    static void PollTaskCB(int a, void* b)
    {
    }
}

static long total_elapsed = 0;
static boost::timer::nanosecond_type const limit(1 * 1000000000LL); // 1 seconds in nanoseconds

PollTask::PollTask( FileManager* pFm,
                    CredentialsManager* pCm,
                    const AccessToken& at,
                    const Entity& entity,
                    const TaskContext& context,
                    TaskDelegate* callbackDelegate)
                    :                                               
                    TentTask(Task::POLL,
                             pFm,
                             pCm,
                             at,
                             entity,                               
                             context,
                             callbackDelegate) {
    delegate_ = new PollDelegate(this);
    running_ = true;
}

PollTask::~PollTask() {
    if(delegate_) {
        delete delegate_;
        delegate_ = NULL;
    }
}

void PollTask::OnStart(){
    std::cout<<" POLL TASK STARTING " << std::endl;
    if(!polltask::g_pCurrentPollTask) {
        polltask::g_pCurrentPollTask = this;
    }

    event::RegisterForEvent(this, event::Event::PAUSE);
    event::RegisterForEvent(this, event::Event::RESUME);
    Entity entity = TentTask::entity();
    census_handler_.Initialize(entity.GetPreferredServer().posts_feed(),
                               entity.GetPreferredServer().post(),
                               access_token());
    timer_.start();
}

void PollTask::OnPaused() {}

void PollTask::OnFinished() {
    std::cout<<" POLL TASK FINISHING " << std::endl;
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
    if(processing_queue_.find(returnpost) != processing_queue_.end()) {
        // remove it from the map
        processing_queue_.erase(returnpost);
    }
    else {
        std::cout<<" POSTID NOT FOUND IN QUEUE ?!?!? " << b << std::endl;
    }
}

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
                if(census_handler_.Inquiry()) {
                    std::cout<<" Syncing files ... " << std::endl;
                    status = SyncFiles();
                    //status = SyncFolderPosts();
                    if(status != ret::A_OK)
                        std::cout<<" POLLING ERR : " << status << std::endl;
                }
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
    std::deque<FolderPost> folders;
    status = RetrieveFolderPosts(folders);
    if(status == ret::A_OK) {
        std::deque<FolderPost>::iterator itr = folders.begin();
        for(;itr != folders.end(); itr++) {
            status = SyncFolder(*itr);
            if(status != ret::A_OK) {
                // Some kind of logging
                std::cout<<" FAILED TO SYNC FOLDER : " << status << std::endl;
            }
        }
    }
    return status;
}

int PollTask::SyncFolder(FolderPost& folder_post) {
    std::cout<<" SYNCING FOLDER " << std::endl;
    int status = ret::A_OK;
    std::deque<FilePost> file_posts;
    status = RetrieveFilePostsThatMentionFolder(folder_post.id(), file_posts);
    if(status == ret::A_OK) {
        std::deque<FilePost>::iterator itr = file_posts.begin();
        for(;itr != file_posts.end(); itr++) {
            if(processing_queue_.find((*itr).id()) == processing_queue_.end()) {
                processing_queue_[(*itr).id()] = true;
                std::cout<<" poll task raise event : id : " << (*itr).id() << std::endl;
                event::RaiseEvent(event::Event::REQUEST_SYNC_POST, 
                                  (*itr).id(),
                                  delegate_);
            }
        }
    }
    else {
        std::cout<<" RETRIEVE FILE POSTS STATUS : " << status << std::endl;

    }
    return status;
}

int PollTask::RetrieveFilePostsThatMentionFolder(const std::string& post_id, 
                                                 std::deque<FilePost>& posts) {
    int status = ret::A_OK;
    std::cout<<" RETRIEVE FILE POSTS " << std::endl;
    int postcount = GetFilePostCount(post_id);
    std::cout<<" FILE POST COUNT : " << postcount << std::endl;
    std::cout<<" POST ID : " << post_id << std::endl;
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
            params.AddValue(std::string("mentions"), entity.entity() + "+" + post_id);
            params.AddValue(std::string("types"), std::string(cnst::g_attic_file_type));  
            params.AddValue(std::string("limit"), std::string(countBuff));                         
            if(!lastid.empty())
                params.AddValue(std::string("last_id"), lastid);

            Response resp;
            netlib::HttpGet(posts_feed,
                            &params,
                            &at,
                            resp);

            std::cout<< "LINK HEADER : " << resp.header["Link"] << std::endl;
            std::cout<<" response code : " << resp.code << std::endl;
            std::cout<<" response body : " << resp.body << std::endl;

            if(resp.code == 200) { 
                Json::Value root;
                Json::Reader reader;
                reader.parse(resp.body, root);
                //jsn::PrintOutJsonValue(&root);

                Json::ValueIterator itr = root.begin();
                for(; itr != root.end(); itr++) {
                    FilePost fp;
                    if(jsn::DeserializeObject(&fp, *itr))
                        posts.push_back(fp);
                }
                lastid = posts[posts.size()-1].id();
            }
            else {
                log::LogHttpResponse("AMM23812", resp);
                status = ret::A_FAIL_NON_200;
                break;
            }
        }
    }
    return status;
}

int PollTask::SyncFiles() {
    int status = ret::A_OK;

    std::deque<FilePost> posts;
    PagePost pp;
    while(RetrieveFilePosts(posts, pp)) {
        std::cout<< "PAGES : \n" << pp.pages().asString() << std::endl;
        sleep::sleep_seconds(1);
        std::cout<<" LAST ID : " << std::endl;

        // Process Posts
        std::deque<FilePost>::iterator itr = posts.begin();
        for(;itr != posts.end(); itr++) {
            if(processing_queue_.find((*itr).id()) == processing_queue_.end()) {
                processing_queue_[(*itr).id()] = true;
                std::cout<<" poll task raise event : id : " << (*itr).id() << std::endl;
                event::RaiseEvent(event::Event::REQUEST_SYNC_POST, 
                                  (*itr).id(),
                                  delegate_);
            }
        }
        posts.clear();
        std::cout<< "NEXT : " << pp.pages().next() << std::endl;
        std::cout<< "EMPTY? " << pp.pages().next().empty() << std::endl;
        if(pp.pages().next().empty()) { 
            break;
        }
    }

    return status;
}

bool PollTask::RetrieveFilePosts(std::deque<FilePost>& posts, PagePost& out) {
    std::cout<< " retrieving file posts " << std::endl;
    Entity entity = TentTask::entity();
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();

    AccessToken at = access_token();

    char count_buf[256] = {'\0'};
    snprintf(count_buf, 256, "%d", 200);

    std::string last_id = out.pages().next();

    std::cout<<" NEXT ID : " << last_id << std::endl;

    UrlParams params;                                                                  
    params.AddValue(std::string("types"), std::string(cnst::g_attic_file_type));  
    params.AddValue(std::string("limit"), std::string(count_buf));
    if(!last_id.empty())
        params.AddValue(std::string("last_id"), last_id);

    Response resp;
    netlib::HttpGet(posts_feed,
                    &params,
                    &at,
                    resp);

    if(resp.code == 200) { 
        jsn::DeserializeObject(&out, resp.body);

        Json::Value arr(Json::arrayValue);
        jsn::DeserializeJson(out.data(), arr);

        std::cout<<" iterating " << std::endl;
        std::cout<<" data : " << out.data() << std::endl;
        Json::ValueIterator itr = arr.begin();
        for(; itr != arr.end(); itr++) {
            FilePost fp;
            if(jsn::DeserializeObject(&fp, *itr))
                posts.push_back(fp);
        }
        return true;
    }
    else {
        log::LogHttpResponse("A0214812", resp);
    }

    return false;
}

int PollTask::RetrieveFolderPosts(std::deque<FolderPost>& posts) {
    int status = ret::A_OK;
    std::cout<<" RETRIEVING FOLDER POSTS " << std::endl;
    int postcount = GetFolderPostCount();
    if(postcount > 0) {
        std::cout<<" FOLDER POST COUNT : " << postcount << std::endl;
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
            params.AddValue(std::string("types"), std::string(cnst::g_attic_folder_type));  
            params.AddValue(std::string("limit"), std::string(countBuff));                         
            if(!lastid.empty())
                params.AddValue(std::string("last_id"), lastid);

            Response resp;
            netlib::HttpGet(posts_feed,
                            &params,
                            &at,
                            resp);

            //std::cout<< "LINK HEADER : " << resp.header["Link"] << std::endl;
            //std::cout<<" response code : " << resp.code << std::endl;
            //std::cout<<" response body : " << resp.body << std::endl;

            if(resp.code == 200) { 
                Json::Value root;
                Json::Reader reader;
                reader.parse(resp.body, root);
                //jsn::PrintOutJsonValue(&root);

                std::cout<<" iterating " << std::endl;
                Json::ValueIterator itr = root.begin();
                for(; itr != root.end(); itr++) {
                    FolderPost fp;
                    if(jsn::DeserializeObject(&fp, *itr))
                        posts.push_back(fp);
                }
                lastid = posts[posts.size()-1].id();
            }
            else {
                log::LogHttpResponse("AMM23812", resp);
                status = ret::A_FAIL_NON_200;
                break;
            }
        }
    }
    return status;
}

int PollTask::GetFilePostCount(const std::string& folder_post_id) {
    Entity entity = TentTask::entity();
    std::string url = entity.GetPreferredServer().posts_feed();

    UrlParams params;
    params.AddValue(std::string("mentions"), entity.entity() + "+" + folder_post_id);
    params.AddValue(std::string("types"), std::string(cnst::g_attic_file_type));

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

int PollTask::GetFolderPostCount() {
    std::string url = entity().GetPreferredServer().posts_feed();
    //std::cout<<" URL : " << url << std::endl;

    UrlParams params;
    params.AddValue(std::string("types"), std::string(cnst::g_attic_folder_type));             

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
    processing_queue_[filepath] = true;
}

void PollTask::RemoveFile(const std::string& filepath) {
    processing_queue_.erase(filepath);
}

bool PollTask::IsFileInQueue(const std::string& filepath) {
    if(processing_queue_.find(filepath)!= processing_queue_.end())
        return true;
    return false;
}


}//namespace
