#ifndef EVENT_H_
#define EVENT_H_
#pragma once

#include <map>
#include <vector>
#include <string>

#include "mutexclass.h"
#include "taskdelegate.h"

namespace attic { namespace event {

struct Event {
    enum EventStatus {
        START = 0,
        RUNNING,
        PAUSED,
        DONE,
    };

    enum EventType {
        PUSH = 0,
        PULL,
        DELETE,
        REQUEST_PUSH,
        REQUEST_PULL,
        REQUEST_DELETE,
        REQUEST_SYNC_POST,
        SYNC,
        POLL,
        UPLOAD_SPEED,
        DOWNLOAD_SPEED,
        FILE_LOCK,
        FILE_UNLOCK,
        RECOVERY_KEY,
        TEMPORARY_PASS,
        ERROR_NOTIFY,
        PAUSE_RESUME_NOTIFY,
        PAUSE,
        RESUME
    };

    EventStatus status = START;
    EventType type;
    std::string value;
    TaskDelegate* delegate = NULL;
};

class EventListener {
public:
    EventListener() {}
    ~EventListener() {}

    virtual void OnEventRaised(const Event& event) = 0;
};

class EventSystem {
    EventSystem() {}
    EventSystem(const EventSystem& rhs) {}
    EventSystem operator=(const EventSystem& rhs) { return *this; }
    void Notify(const Event& event);
public:
    ~EventSystem() {}

    static EventSystem* GetInstance();

    void Initialize() {}
    void Shutdown();

    void ProcessEvents();

    void RaiseEvent(const Event& event);

    void RegisterForEvent(EventListener* pListener, Event::EventType type);
    void UnregisterFromEvent(const EventListener* pListener, Event::EventType type);

private:
    typedef std::deque<Event> EventQueue;
    typedef std::vector<EventListener*> Listeners;
    typedef std::map<Event::EventType, Listeners> ListenerMap;

    MutexClass m_listenMtx;
    ListenerMap m_ListenerMap;

    MutexClass m_queueMtx;
    EventQueue m_EventQueue;

    static EventSystem* m_pInstance;
};


static void RegisterForEvent(EventListener* pListener, Event::EventType type) {
    if(pListener) {
        EventSystem::GetInstance()->RegisterForEvent(pListener, type);  
    }
}

static void UnregisterFromEvent(EventListener* pListener, Event::EventType type) {
    if(pListener) {
        EventSystem::GetInstance()->UnregisterFromEvent(pListener, type);  
    }

}

static void RaiseEvent(const Event& event) {
    EventSystem::GetInstance()->RaiseEvent(event);
}

static void RaiseEvent(const Event::EventType type, 
                       const std::string& value, 
                       TaskDelegate* delegate) {
    Event event;
    event.type = type;
    event.value = value;
    event.delegate = delegate;
    RaiseEvent(event);
}

static void RaiseEvent(const Event::EventType type,
                       const Event::EventStatus status,
                       const std::string& value,
                       TaskDelegate* delegate) {
    Event event;
    event.type = type;
    event.status = status;
    event.value = value;
    event.delegate = delegate;
    RaiseEvent(event);
}


static void ShutdownEventSystem() {
    EventSystem::GetInstance()->Shutdown();
}

}}//namesapce
#endif

