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
        UPLOAD_SPEED,
        DOWNLOAD_SPEED
    };

    EventType type;
    std::string value;
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

    EventSystem* GetInstance();

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

#endif

