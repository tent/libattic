#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_
#pragma once

#include <map>
#include <deque>
#include "eventsystem.h"
#include "eventsystem.h"

class CallbackHandler : public EventListener {

    void Notify(const Event& event);
public:
    typedef void(*EventCallback)(int, int, const char*);

    CallbackHandler();
    ~CallbackHandler();

    void Initialize();

    void RegisterCallback(Event::EventType type, EventCallback cb);
    void OnEventRaised(const Event& event);

private:
    typedef std::deque<EventCallback> CallbackList;
    std::map<Event::EventType, CallbackList>  m_CallbackMap;

};

#endif




