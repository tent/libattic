#include "callbackhandler.h"

namespace attic { 

CallbackHandler::CallbackHandler() {}
CallbackHandler::~CallbackHandler() {}

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
        std::string id = del->GenerateIdentifier();
        while(delegate_map_.find(id) != delegate_map_.end())
            id = del->GenerateIdentifier();
        delegate_map_[id] = del;
    }
    return del;
}

void RemoveDelegate(const std::string& id);

} //namespace

