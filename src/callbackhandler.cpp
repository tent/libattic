#include "callbackhandler.h"

CallbackHandler::CallbackHandler() {}
CallbackHandler::~CallbackHandler() {}

void CallbackHandler::Initialize() {
    EventSystem::GetInstance()->RegisterForEvent(this, Event::PULL);
    EventSystem::GetInstance()->RegisterForEvent(this, Event::PUSH);
    EventSystem::GetInstance()->RegisterForEvent(this, Event::UPLOAD_SPEED);
    EventSystem::GetInstance()->RegisterForEvent(this, Event::DOWNLOAD_SPEED);
}

void CallbackHandler::RegisterCallback(Event::EventType type, EventCallback cb) {
    m_CallbackMap[type].push_back(cb);
}

void CallbackHandler::OnEventRaised(const Event& event) {
    Notify(event);
}

void CallbackHandler::Notify(const Event& event) {
    CallbackList::iterator itr = m_CallbackMap[event.type].begin();
    for(;itr!=m_CallbackMap[event.type].end(); itr++) {
        if(*itr) 
            (*itr)(event.type, event.status, event.value.c_str());
    }

}

