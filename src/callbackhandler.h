#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_
#pragma once

#include <map>
#include <deque>
#include "event.h"
#include "mutexclass.h"
#include "taskdelegate.h"

namespace attic { 

class CallbackHandler : public event::EventListener {
    void Notify(const event::Event& event);
    void InsertDelegateIntoMap(TaskDelegate* del);
public:
    typedef void(*EventCallback)(int, int, const char*);

    CallbackHandler();
    ~CallbackHandler();

    void RegisterCallback(event::Event::EventType type, EventCallback cb);
    void OnEventRaised(const event::Event& event);

    TaskDelegate* RegisterDelegateCallback(int type, cbh::DelegateCallback cb);
    TaskDelegate* RegisterManifestCallback(cbh::QueryCallback cb);
    TaskDelegate* RegisterFileHistoryCallback(cbh::HistoryCallback cb);
    void RemoveDelegate(const std::string& id);
private:
    typedef std::deque<EventCallback> CallbackList;
    std::map<event::Event::EventType, CallbackList>  callback_map_;
    std::map<std::string, TaskDelegate*>             delegate_map_;

    MutexClass del_mtx_;
    MutexClass cbm_mtx_;
};

} //namespace
#endif
