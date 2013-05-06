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
//static boost::timer::nanosecond_type const limit(1 * 1000000000LL); // 1 seconds in nanoseconds
static boost::timer::nanosecond_type const limit(1 * 100000000LL); 

PollTask::PollTask( FileManager* pFm,
                    CredentialsManager* pCm,
                    const AccessToken& at,
                    const Entity& entity,
                    const TaskContext& context)
                    :                                               
                    TentTask(Task::POLL,
                             pFm,
                             pCm,
                             at,
                             entity,                               
                             context) {
    census_handler_ = NULL;    
    delegate_ = new PollDelegate(this);
    running_ = true;
}

PollTask::~PollTask() {
    if(delegate_) {
        delete delegate_;
        delegate_ = NULL;
    }
    if(census_handler_) {
        delete census_handler_;
        census_handler_ = NULL;
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
    census_handler_ = new CensusHandler(entity.GetPreferredServer().posts_feed(), access_token());
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
    std::cout<<" POLL TASK CALLBACK HIT " << std::endl;
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
                std::cout<<" processing queue size : " << processing_queue_.size() << std::endl;
                std::deque<FilePost> file_list;
                if(census_handler_->Inquiry(file_list)) {
                    std::cout<<" Syncing files ... " << std::endl;
                    status = SyncFiles(file_list);
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

int PollTask::SyncFiles(std::deque<FilePost>& file_list) {
    int status = ret::A_OK;
    // Process Posts
    std::deque<FilePost>::iterator itr = file_list.begin();
    for(;itr != file_list.end(); itr++) {
        if(processing_queue_.find((*itr).id()) == processing_queue_.end()) {
            processing_queue_[(*itr).id()] = true;
            std::cout<<" poll task raise event : id : " << (*itr).id() << std::endl;
            event::RaiseEvent(event::Event::REQUEST_SYNC_POST, 
                              (*itr).id(),
                              delegate_);
        }
    }
    return status;
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
