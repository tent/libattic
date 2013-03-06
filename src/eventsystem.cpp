#include "eventsystem.h"

EventSystem* EventSystem::m_pInstance = 0;

EventSystem* EventSystem::GetInstance() {
    if(!m_pInstance)
        m_pInstance = new EventSystem();
    return m_pInstance;
}

void EventSystem::Shutdown()
{
    Lock();
    ListenerMap::iterator itr = m_ListenerMap.begin();
    for(;itr != m_ListenerMap.end(); itr++) {
        itr->second.clear();
    }
    m_ListenerMap.clear();

    if(m_pInstance) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
    Unlock();
}
void EventSystem::RaiseEvent(const Event& event)
{
    // TODO:: test the cost memory vs speed making local copy of map or vector and iterating through that
    // Notify listeners
    Lock();
    if(m_ListenerMap[event.type].size()) {
        Listeners::iterator itr = m_ListenerMap[event.type].begin();
        for(;itr != m_ListenerMap[event.type].end(); itr++) {
            if(*itr) 
                (*itr)->OnEventRaised(event);
        }
    }
    Unlock();
}

void EventSystem::RegisterForEvent(EventListener* pListener, Event::EventType type)
{
    Lock();
    m_ListenerMap[type].push_back(pListener);
    Unlock();
}

void EventSystem::UnregisterFromEvent(const EventListener* pListener, Event::EventType type)
{
    Lock();
    Listeners::iterator itr = m_ListenerMap[type].begin();
    for(;itr != m_ListenerMap[type].end(); itr++) {
        if(pListener == *itr) {
            m_ListenerMap[type].erase(itr);
            break;
        }
    }
    Unlock();
}


