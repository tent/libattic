#include "taskdelegate.h"

#include "callbackhandler.h"
#include "taskcontext.h"

namespace attic {

HistoryDelegate::HistoryDelegate(CallbackHandler* handler, 
                                 cbh::HistoryCallback cb) : TaskDelegate(TaskDelegate::FILEHISTORY) {
    cb_ = cb;
    owner_ = handler; 
}

void HistoryDelegate::Callback(const int type,
                               const int code,
                               const int state,
                               const std::string& var) const {
    // callback 
    // remove self 
    if(owner_) owner_->RemoveDelegate(identifier());
} 

void HistoryDelegate::Callback(const int code, 
                               const char* buffer,
                               const int stride,
                               const int total) {
    if(cb_) cb_(code, buffer, stride, total);
    if(owner_) owner_->RemoveDelegate(identifier());
}

ManifestDelegate::ManifestDelegate(CallbackHandler* handler, 
                                   cbh::QueryCallback cb) : TaskDelegate(TaskDelegate::MANIFEST) {
    cb_ = cb;
    owner_ = handler; 
}

void ManifestDelegate::Callback(const int type,
                                const int code,
                                const int state,
                                const std::string& var) const {
    // callback 
    // remove self 
    if(owner_) owner_->RemoveDelegate(identifier());
}

void ManifestDelegate::Callback(const int code, 
                                char** buffer,
                                const int stride,
                                const int total) {
    if(cb_) cb_(code, buffer, stride, total);
    if(owner_) owner_->RemoveDelegate(identifier());
}

RequestDelegate::RequestDelegate(CallbackHandler* handler, 
                                 cbh::RequestCallback cb) : TaskDelegate(TaskDelegate::REQUEST) {
    cb_ = cb;
    owner_ = handler; 
}

void RequestDelegate::Callback(const int type,
                               const int code,
                               const int state,
                               const std::string& var) const {
    // callback 
    // remove self 
    if(owner_) owner_->RemoveDelegate(identifier());
}

void RequestDelegate::Callback(const int code, 
                               const char* payload,
                               const char* error) {
    if(cb_) cb_(code, payload, error);
    if(owner_) owner_->RemoveDelegate(identifier());
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

