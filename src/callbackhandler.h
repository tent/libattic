#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_
#pragma once

#include <map>
#include <deque>
#include "event.h"

class CallbackHandler : public event::EventListener {
    void Notify(const event::Event& event);
public:
    typedef void(*EventCallback)(int, int, const char*);

    CallbackHandler();
    ~CallbackHandler();

    void Initialize();

    void RegisterCallback(event::Event::EventType type, EventCallback cb);
    void OnEventRaised(const event::Event& event);

private:
    typedef std::deque<EventCallback> CallbackList;
    std::map<event::Event::EventType, CallbackList>  m_CallbackMap;
};

#endif




