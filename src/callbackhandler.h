#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_
#pragma once

#include <map>
#include <deque>
#include "eventsystem.h"
#include "eventsystem.h"

class CallbackHandler : public EventListener {
public:
    typedef void(*EventCallback)(int, char*);

    CallbackHandler() {}
    ~CallbackHandler() {}

    void RegisterCallback(Event::EventType type, EventCallback cb) {
        m_CallbackMap[type].push_back(cb);
    }

    void OnEventRaised(const Event& event) {
        switch(event.type) {
            default:
                break;
        };

    }

private:
    typedef std::deque<EventCallback> CallbackList;
    std::map<Event::EventType, CallbackList>  m_CallbackMap;

};
#endif

