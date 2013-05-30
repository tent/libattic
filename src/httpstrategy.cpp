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
int HttpStrategyInterface::InitInstance(FileManager* fm,
                                        CredentialsManager* cm) {
    int status = ret::A_OK;
    file_manager_ = fm;
    credentials_manager_ = cm;
    if(!file_manager_) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    if(!credentials_manager_) return ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    credentials_manager_->GetAccessTokenCopy(access_token_);
    return status;
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

HttpStrategyContext::HttpStrategyContext(FileManager* fm,
                                         CredentialsManager* cm) {
    file_manager_ = fm;
    credentials_manager_ = cm;
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

int HttpStrategyContext::Execute(HttpStrategyInterface* s) {
    if(s) {
        // Copy context config
        s->config_map_ = config_map_;
        // Execute
        return s->Execute(file_manager_,
                          credentials_manager_);
    }
    return ret::A_FAIL_INVALID_PTR;
}

int HttpStrategyContext::ExecuteAll() {
    // Check config for anything interesting
    StrategyList::iterator itr = strategies_.begin();
    int status = ret::A_OK;
    for(;itr != strategies_.end(); itr++) {
        status = Execute(*itr);
        if(status != ret::A_OK)
            break;
    }
    return status;
}

int HttpStrategyContext::Step() {
    int status = ret::A_OK;
    if(strategy_itr_ != strategies_.end()) {
        status = Execute(*strategy_itr_);
        strategy_itr_++;
    }
    return status;
}

void HttpStrategyContext::ResetPosition() {
    strategy_itr_ = strategies_.begin();
}

}//namespace
