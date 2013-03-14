#include "callbackhandler.h"

CallbackHandler::CallbackHandler() {}
CallbackHandler::~CallbackHandler() {}

void CallbackHandler::Initialize() {
    event::RegisterForEvent(this, event::Event::PULL);
    event::RegisterForEvent(this, event::Event::PUSH);
    event::RegisterForEvent(this, event::Event::UPLOAD_SPEED);
    event::RegisterForEvent(this, event::Event::DOWNLOAD_SPEED);
    event::RegisterForEvent(this, event::Event::ERROR_NOTIFY);
}

void CallbackHandler::RegisterCallback(event::Event::EventType type, EventCallback cb) {
    m_CallbackMap[type].push_back(cb);
}

void CallbackHandler::OnEventRaised(const event::Event& event) {
    Notify(event);
}

void CallbackHandler::Notify(const event::Event& event) {
    CallbackList::iterator itr = m_CallbackMap[event.type].begin();
    for(;itr!=m_CallbackMap[event.type].end(); itr++) {
        if(*itr) 
            (*itr)(event.type, event.status, event.value.c_str());
    }

}

