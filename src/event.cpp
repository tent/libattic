#include "event.h"

namespace attic { namespace event {

EventSystem* EventSystem::m_pInstance = 0;

EventSystem* EventSystem::GetInstance() {
    if(!m_pInstance)
        m_pInstance = new EventSystem();
    return m_pInstance;
}

void EventSystem::Shutdown() {
    m_listenMtx.Lock();
    ListenerMap::iterator itr = m_ListenerMap.begin();
    for(;itr != m_ListenerMap.end(); itr++) {
        itr->second.clear();
    }
    m_ListenerMap.clear();
    m_listenMtx.Unlock();
    m_queueMtx.Lock();
    m_EventQueue.clear();
    m_queueMtx.Unlock();

    if(m_pInstance) {
        delete m_pInstance;
        m_pInstance = NULL;
    }

}

void EventSystem::ProcessEvents() {

    m_queueMtx.Lock();
    unsigned int count = m_EventQueue.size();
    m_queueMtx.Unlock();

    std::deque<Event> tempQueue;
    unsigned int stride = 5;
    for(unsigned int i=0; i<count; i+=stride){
        // Copy some events
        m_queueMtx.Lock();
        for(unsigned int b = 0; b<stride; b++) {
            if(m_EventQueue.size()) {
                tempQueue.push_back(m_EventQueue.front());
                m_EventQueue.pop_front();
            }
            else {
                break;
            }
        }
        m_queueMtx.Unlock();

        //Process Temp events
        std::deque<Event>::iterator itr = tempQueue.begin();
        for(;itr!= tempQueue.end(); itr++)
            Notify(*itr);
        tempQueue.clear();
        
    }

}

void EventSystem::Notify(const Event& event) {
    // Notify listeners
    m_listenMtx.Lock();
    if(m_ListenerMap[event.type].size()) {
        Listeners::iterator itr = m_ListenerMap[event.type].begin();
        for(;itr != m_ListenerMap[event.type].end(); itr++) {
            if(*itr)
                (*itr)->OnEventRaised(event);
        }
    }
    m_listenMtx.Unlock();

}

void EventSystem::RaiseEvent(const Event& event) {
    // Pushback
    m_queueMtx.Lock();
    m_EventQueue.push_back(event);
    m_queueMtx.Unlock();

    //Notify(event);
}

void EventSystem::RegisterForEvent(EventListener* pListener, Event::EventType type) {
    m_listenMtx.Lock();
    m_ListenerMap[type].push_back(pListener);
    m_listenMtx.Unlock();
}

void EventSystem::UnregisterFromEvent(const EventListener* pListener, Event::EventType type) {
    m_listenMtx.Lock();
    Listeners::iterator itr = m_ListenerMap[type].begin();
    for(;itr != m_ListenerMap[type].end(); itr++) {
        if(pListener == *itr) {
            m_ListenerMap[type].erase(itr);
            break;
        }
    }
    m_listenMtx.Unlock();
}

}} //namespace
