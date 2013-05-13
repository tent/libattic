#include "taskdispatch.h"

#include <iostream>

#include "errorcodes.h"
#include "taskfactory.h"
#include "taskarbiter.h"
#include "filemanager.h"
#include "taskqueue.h"


namespace attic { 

TaskDispatch::TaskDispatch(FileManager* fm,
                           CredentialsManager* cm,
                           const AccessToken& at,
                           const Entity& entity,
                           const std::string& tempdir, 
                           const std::string& workingdir,
                           const std::string& configdir) {
    file_manager_ = fm;
    credentials_manager_ = cm;

    access_token_ = at;
    entity_ = entity;

    temp_directory_ = tempdir;
    working_directory_ = workingdir;
    config_directory_ = configdir;
}

TaskDispatch::~TaskDispatch() {
    file_manager_ = NULL;
    credentials_manager_ = NULL;
}
void TaskDispatch::Initialize() {}
void TaskDispatch::Shutdown() {}

/*
void TaskDispatch::Process(TaskManager* tm) {
    // Grab all the current contexts
    if(tm) {
        tm->RetrieveContextQueue(hold_queue_);
    }
    // Distribute contexts
    if(hold_queue_.size() > 0) {
        std::cout<<" dispatch hold queue size : " << hold_queue_.size() << std::endl;

        TaskContext::ContextQueue::iterator itr = hold_queue_.begin();
        for(;itr != hold_queue_.end(); itr++) {
            std::cout<<" copying context " << std::endl;
            task_map_[(*itr).type()].push_back(*itr);
        }
        hold_queue_.clear();
    }
}

void TaskDispatch::Dispatch() {
    unsigned int task_count = TaskArbiter::GetInstance()->ActiveTaskCount();
    if(task_count < 5) { // TODO :: testing, up this limit
        unsigned int t =0;
        TaskMap::iterator itr = task_map_.begin();
        for(;itr != task_map_.end(); itr++) {
            std::deque<TaskContext>::iterator d_itr = task_map_[itr->first].begin();
            for(; d_itr != task_map_[itr->first].end(); d_itr++) {
                CreateAndSpinOffTask(*d_itr);
            }
            //task_map_.erase(itr);

       }
        task_map_.clear();
    }
    else {
        std::cout<<" Dispatch , task count : " << task_count << std::endl;
        std::cout<<" dispatch queue count : " <<task_map_.size() << std::endl;
    }
}


int TaskDispatch::CreateAndSpinOffTask(const TaskContext& tc) {
    std::cout<<" creating and spinning of task, type : " << tc.type() << std::endl;
    int status = TaskArbiter::GetInstance()->CreateAndSpinOffTask(tc);
    return status;
}
*/

} //namespace

