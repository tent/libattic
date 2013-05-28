#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_
#pragma once

#include <map>
#include <deque>
#include "event.h"

#include "taskdelegate.h"

namespace attic { 

class CallbackHandler : public TaskDelegate, public event::EventListener {
    void Notify(const event::Event& event);
public:
    typedef void(*EventCallback)(int, int, const char*);

    CallbackHandler();
    ~CallbackHandler();

    void RegisterCallback(event::Event::EventType type, EventCallback cb);
    void OnEventRaised(const event::Event& event);


    typedef void(*DelegateCallback)(int, int, const char*);
    void RegisterDelegateCallback(int type, DelegateCallback cb);
    void Callback(const int type,
                  const int code,
                  const int state,
                  const std::string& var) const;
private:
    typedef std::deque<EventCallback> CallbackList;
    std::map<event::Event::EventType, CallbackList>  callback_map_;
    std::map<int, DelegateCallback>             delegate_map_;
};

} //namespace
#endif
