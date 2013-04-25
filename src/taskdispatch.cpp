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


void TaskDispatch::Process(TaskManager* tm) {
    if(tm) {
        tm->RetrieveContextQueue(hold_queue_);
    }
    // Copy n, number to process queue, then process that
    dispatch_queue_ = hold_queue_; // TODO :: actually mitigate this properly, this is for testing
    hold_queue_.clear();
}

void TaskDispatch::Dispatch() {
    unsigned int task_count = CentralTaskQueue::GetInstance()->TaskCount();
    if(task_count < 20) {
        unsigned int t =0;
        TaskContext::ContextQueue::iterator itr = dispatch_queue_.begin();
        for(;itr != dispatch_queue_.end(); itr++) {
            CreateAndSpinOffTask(*itr);
            t++;
            if((t+task_count) > 50)
                break;
        }
    }
}

int TaskDispatch::CreateAndSpinOffTask(const TaskContext& tc) {
    int status = ret::A_OK;
    Task* t = task_factory_.GetTentTask(tc.type(),
                                        file_manager_,
                                        credentials_manager_,
                                        access_token_,
                                        entity_,
                                        tc, 
                                        tc.delegate(),
                                        NULL);

    status = TaskArbiter::GetInstance()->SpinOffTask(t);
    return status;
}

} //namespace

