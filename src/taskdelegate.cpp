#include "taskdelegate.h"

#include "callbackhandler.h"

namespace attic {

HistoryCallback::HistoryCallback(CallbackHandler* handler, 
                                 cbh::HistoryCallback cb) : TaskDelegate(TaskDelegate::FILEHISTORY) {
    cb_ = cb;
    owner_ = handler; 
}

void HistoryCallback::Callback(const int type,
                               const int code,
                               const int state,
                               const std::string& var) const {
    // callback 
    // remove self 
    if(owner_)
        owner_->RemoveDelegate(identifier());

} 

void HistoryCallback::Callback(const int code, 
                               const char* buffer,
                               const int stride,
                               const int total) {
    if(cb_)
        cb_(code, buffer, stride, total);
    if(owner_)
        owner_->RemoveDelegate(identifier());
}

ManifestCallback::ManifestCallback(CallbackHandler* handler, 
                                   cbh::QueryCallback cb) : TaskDelegate(TaskDelegate::MANIFEST) {
    cb_ = cb;
    owner_ = handler; 
}

void ManifestCallback::Callback(const int type,
                                const int code,
                                const int state,
                                const std::string& var) const {
    // callback 
    // remove self 
    if(owner_)
        owner_->RemoveDelegate(identifier());
}

void ManifestCallback::Callback(const int code, 
                                char** buffer,
                                const int stride,
                                const int total) {
    if(cb_)
        cb_(code, buffer, stride, total);
    if(owner_)
        owner_->RemoveDelegate(identifier());
}

TaskCallback::TaskCallback(CallbackHandler* handler, 
                           cbh::DelegateCallback cb) : TaskDelegate(TaskDelegate::TASK) {
    cb_ = cb;
    owner_ = handler;
}

void TaskCallback::Callback(const int type,
                            const int code,
                            const int state,
                            const std::string& var) const {
    // callback 
    if(cb_)
        cb_(type, var.size(), var.c_str());  
    // remove self 
    if(owner_)
        owner_->RemoveDelegate(identifier());
}

} // namespace 

