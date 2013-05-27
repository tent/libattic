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

void CallbackHandler::Callback(const int type,
                               const int code,
                               const int state,
                               const std::string& var) {
    if(delegate_map_[type])
        delegate_map_[type](type, code, var.c_str());
}

void CallbackHandler::RegisterDelegateCallback(int type, DelegateCallback cb) {
    delegate_map_[type] = cb;
}

} //namespace

