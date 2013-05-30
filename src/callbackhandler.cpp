#include "callbackhandler.h"

namespace attic { 

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

CallbackHandler::CallbackHandler() {}
CallbackHandler::~CallbackHandler() {
    std::map<std::string, TaskDelegate*>::iterator itr = delegate_map_.begin();
    for(;itr != delegate_map_.end(); itr++) {
        if(itr->second) {
            delete itr->second;
            itr->second = NULL;
        }
    }
    delegate_map_.clear();
}

void CallbackHandler::RegisterCallback(event::Event::EventType type, EventCallback cb) {
    event::RegisterForEvent(this, type);
    callback_map_[type].push_back(cb);
}

void CallbackHandler::OnEventRaised(const event::Event& event) {
    std::cout<<" Notifying : " << event.type << std::endl;
    Notify(event);
}

void CallbackHandler::Notify(const event::Event& event) {
    CallbackList::iterator itr = callback_map_[event.type].begin();
    for(;itr!=callback_map_[event.type].end(); itr++) {
        if(*itr) {
            (*itr)(event.type, event.status, event.value.c_str());
        }
    }

}

TaskDelegate* CallbackHandler::RegisterDelegateCallback(int type, cbh::DelegateCallback cb) {
    TaskDelegate* del = NULL;
    if(cb) {
        del = new TaskCallback(this, cb);
        InsertDelegateIntoMap(del);
    }
    return del;
}

TaskDelegate* CallbackHandler::RegisterManifestCallback(cbh::QueryCallback cb) {
    TaskDelegate* del = NULL;
    if(cb) {
        del = new ManifestCallback(this, cb);
        InsertDelegateIntoMap(del);
    }
    return del;
}

void CallbackHandler::InsertDelegateIntoMap(TaskDelegate* del) {
    std::string id = del->GenerateIdentifier();
    while(delegate_map_.find(id) != delegate_map_.end())
        id = del->GenerateIdentifier();
    delegate_map_[id] = del;
}

void CallbackHandler::RemoveDelegate(const std::string& id) {
    std::map<std::string, TaskDelegate*>::iterator itr = delegate_map_.find(id);
    if(itr != delegate_map_.end()) {
        TaskDelegate* p = itr->second;
        itr->second = NULL;
        delegate_map_.erase(itr);
        if(p) {
            delete p; 
            p = NULL;
        }
    }
}

} //namespace

