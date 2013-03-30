#include "httpstrategy.h"

namespace attic { 

HttpStrategyInterface::HttpStrategyInterface() {
    credentials_manager_ = NULL;
    file_manager_ = NULL;
    task_delegate_ = NULL;
}

HttpStrategyInterface::~HttpStrategyInterface() {
    credentials_manager_ = NULL;
    file_manager_ = NULL;
    if(task_delegate_) {
        delete task_delegate_;
        task_delegate_ = NULL;
    }
}

void HttpStrategyInterface::set_task_delegate(TaskDelegate* del) {
    if(task_delegate_) {
        delete task_delegate_;
        task_delegate_ = NULL;
    }
    task_delegate_ = del;
}

void HttpStrategyInterface::Callback(const int tasktype, 
                                     const int code, 
                                     const int taskstate, 
                                     const std::string& var) {
    if(task_delegate_)
        task_delegate_->Callback(tasktype, code, taskstate, var);
}

HttpStrategyContext::HttpStrategyContext(FileManager* pFileManager,
                                         CredentialsManager* pCredentialsManager) {
    file_manager_ = pFileManager;
    credentials_manager_ = pCredentialsManager;
    strategy_itr_ = strategies_.begin();
}

HttpStrategyContext::~HttpStrategyContext() {
    file_manager_ = NULL;
    credentials_manager_ = NULL;
    strategies_.clear();
}

void HttpStrategyContext::PushBack(HttpStrategyInterface* pStrat) { 
    strategies_.push_back(pStrat);
}

int HttpStrategyContext::Execute(HttpStrategyInterface* s, Response& out) {
    if(s) {
        // Copy context config
        s->config_map_ = config_map_;
        // Execute
        return s->Execute(file_manager_,
                          credentials_manager_,
                          out);
    }
    return ret::A_FAIL_INVALID_PTR;
}

int HttpStrategyContext::ExecuteAll() {
    // Check config for anything interesting
    StrategyList::iterator itr = strategies_.begin();
    Response resp;
    int status = ret::A_OK;
    for(;itr != strategies_.end(); itr++) {
        status = Execute(*itr, resp);
        if(status != ret::A_OK)
            break;
    }
    return status;
}

int HttpStrategyContext::Step(Response& out) {
    int status = ret::A_OK;
    if(strategy_itr_ != strategies_.end()) {
        status = Execute(*strategy_itr_, out);
        strategy_itr_++;
    }
    return status;
}

void HttpStrategyContext::ResetPosition() {
    strategy_itr_ = strategies_.begin();
}

}//namespace
