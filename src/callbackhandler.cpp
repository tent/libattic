#include "callbackhandler.h"

namespace attic { 

CallbackHandler::CallbackHandler() {}
CallbackHandler::~CallbackHandler() {}

void CallbackHandler::RegisterCallback(event::Event::EventType type, EventCallback cb) {
    event::RegisterForEvent(this, type);
    m_CallbackMap[type].push_back(cb);
}

void CallbackHandler::OnEventRaised(const event::Event& event) {
    std::cout<<" Notifying : " << event.type << std::endl;
    Notify(event);
}

void CallbackHandler::Notify(const event::Event& event) {
    CallbackList::iterator itr = m_CallbackMap[event.type].begin();
    for(;itr!=m_CallbackMap[event.type].end(); itr++) {
        if(*itr) {
            (*itr)(event.type, event.status, event.value.c_str());
        }
    }

}

} //namespace
