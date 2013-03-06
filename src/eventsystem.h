#ifndef EVENTSYSTEM_H_
#define EVENTSYSTEM_H_
#pragma once

#include <map>
#include <vector>
#include <string>

#include "mutexclass.h"

struct Event
{
    enum EventType {
        PUSH = 0,
        PULL,
        REQUEST_PUSH,
        REQUEST_PULL,
        SYNC,
        POLL,
        UPLOAD_SPEED,
        DOWNLOAD_SPEED
    };

    EventType type;
    std::string value;
    void (*callback)(int, void*);
};

class EventListener
{
public:
    EventListener() {}
    ~EventListener() {}

    virtual void OnEventRaised(const Event& event) = 0;
};

class EventSystem : public MutexClass
{
    EventSystem() {}
    EventSystem(const EventSystem& rhs) {}
    EventSystem operator=(const EventSystem& rhs) { return *this; }
public:
    ~EventSystem() {}

    static EventSystem* GetInstance();

    void Shutdown();

    void RaiseEvent(const Event& event);

    void RegisterForEvent(EventListener* pListener, Event::EventType type);
    void UnregisterFromEvent(const EventListener* pListener, Event::EventType type);

private:
    typedef std::vector<EventListener*> Listeners;
    typedef std::map<Event::EventType, Listeners> ListenerMap;

    ListenerMap m_ListenerMap;

    static EventSystem* m_pInstance;
};

namespace evnt
{
    static void RaiseEvent(const Event& event) 
    {
        EventSystem::GetInstance()->RaiseEvent(event);
    }

    static void ShutdownEventSystem()
    {
        EventSystem::GetInstance()->Shutdown();
    }

};

#endif

