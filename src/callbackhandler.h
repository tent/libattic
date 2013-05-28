#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_
#pragma once

#include <map>
#include <deque>
#include "event.h"

#include "taskdelegate.h"

namespace attic { 

namespace cbh {
    typedef void(*DelegateCallback)(int, int, const char*);
    typedef void(*QueryCallback)(int, char**, int, int);
};

class CallbackHandler;

class ManifestCallback : public TaskDelegate { 
public:
    ManifestCallback(CallbackHandler* handler, cbh::QueryCallback cb);

    void Callback(const int type,
                  const int code,
                  const int state,
                  const std::string& var) const;

    void Callback(const int code, 
                  char** buffer,
                  const int stride,
                  const int total); 
private:
    cbh::QueryCallback cb_;
    CallbackHandler* owner_;
};

class TaskCallback : public TaskDelegate {
public:
    TaskCallback(CallbackHandler* handler, cbh::DelegateCallback cb);
    void Callback(const int type,
                  const int code,
                  const int state,
                  const std::string& var) const;
private:
    cbh::DelegateCallback cb_;
    CallbackHandler* owner_;
}; 

class CallbackHandler : public event::EventListener {
    void Notify(const event::Event& event);
public:
    typedef void(*EventCallback)(int, int, const char*);

    CallbackHandler();
    ~CallbackHandler();

    void RegisterCallback(event::Event::EventType type, EventCallback cb);
    void OnEventRaised(const event::Event& event);

    TaskDelegate* RegisterDelegateCallback(int type, cbh::DelegateCallback cb);
    void RemoveDelegate(const std::string& id);
private:
    typedef std::deque<EventCallback> CallbackList;
    std::map<event::Event::EventType, CallbackList>  callback_map_;
    std::map<std::string, TaskDelegate*>             delegate_map_;
};

} //namespace
#endif
