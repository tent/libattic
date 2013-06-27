#include "callbackhandler.h"

namespace attic { 


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
    cbm_mtx_.Lock();
    callback_map_[type].push_back(cb);
    cbm_mtx_.Unlock();
}

void CallbackHandler::OnEventRaised(const event::Event& event) {
    std::cout<<" Notifying : " << event.type << std::endl;
    Notify(event);
}

void CallbackHandler::Notify(const event::Event& event) {
    cbm_mtx_.Lock();
    CallbackList::iterator itr = callback_map_[event.type].begin();
    for(;itr!=callback_map_[event.type].end(); itr++) {
        if(*itr) {
            (*itr)(event.type, event.status, event.value.c_str());
        }
    }
    cbm_mtx_.Unlock();
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
        del = new ManifestDelegate(this, cb);
        InsertDelegateIntoMap(del);
    }
    return del;
}

TaskDelegate* CallbackHandler::RegisterFileHistoryCallback(cbh::HistoryCallback cb) {
    TaskDelegate* del = NULL;
    if(cb) {
        del = new HistoryDelegate(this, cb);
        InsertDelegateIntoMap(del);
    }
    return del;
}

TaskDelegate* CallbackHandler::RegisterRequestCallback(cbh::RequestCallback cb) {
    TaskDelegate* del = NULL;
    if(cb) {
        del = new RequestDelegate(this, cb);
        InsertDelegateIntoMap(del);
    }
    return del;
}

void CallbackHandler::InsertDelegateIntoMap(TaskDelegate* del) {
    std::string id = del->GenerateIdentifier();
    del_mtx_.Lock();
    while(delegate_map_.find(id) != delegate_map_.end())
        id = del->GenerateIdentifier();
    delegate_map_[id] = del;
    del_mtx_.Unlock();
}

void CallbackHandler::RemoveDelegate(const std::string& id) {
    del_mtx_.Lock();
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
    del_mtx_.Unlock();
}

} //namespace

