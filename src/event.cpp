#include "event.h"

namespace attic { namespace event {

EventSystem* EventSystem::instance_ = 0;

EventSystem* EventSystem::instance() {
    if(!instance_)
        instance_ = new EventSystem();
    return instance_;
}

void EventSystem::Shutdown() {
    listen_mtx_.Lock();
    ListenerMap::iterator itr = listener_map_.begin();
    for(;itr != listener_map_.end(); itr++) {
        itr->second.clear();
    }
    listener_map_.clear();
    listen_mtx_.Unlock();
    queue_mtx_.Lock();
    event_queue_.clear();
    queue_mtx_.Unlock();

    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }

}

void EventSystem::ProcessEvents() {
    queue_mtx_.Lock();
    unsigned int count = event_queue_.size();
    queue_mtx_.Unlock();

    if(count) {
        std::cout<<" PROCESSING " << count << " EVENTS " << std::endl;
        std::deque<Event> tempQueue;
        unsigned int stride = 100;
        for(unsigned int i=0; i<count; i+=stride){
            // Copy some events
            queue_mtx_.Lock();

            unsigned int b = 0;
            EventQueue::iterator e_itr = event_queue_.begin();
            for(;e_itr != event_queue_.end(); e_itr++) {
                tempQueue.push_back(*e_itr);
                b++;
                if(b>=stride) break;
            }
            event_queue_.erase(event_queue_.begin(), event_queue_.begin()+b);
            queue_mtx_.Unlock();

            //Process Temp events
            std::deque<Event>::iterator itr = tempQueue.begin();
            for(;itr!= tempQueue.end(); itr++)
                Notify(*itr);
            tempQueue.clear();
        }
    }
}

void EventSystem::Notify(const Event& event) {
    // Notify listeners
    listen_mtx_.Lock();
    if(listener_map_[event.type].size()) {
        Listeners::iterator itr = listener_map_[event.type].begin();
        for(;itr != listener_map_[event.type].end(); itr++) {
            if(*itr)
                (*itr)->OnEventRaised(event);
        }
    }
    listen_mtx_.Unlock();

}

void EventSystem::RaiseEvent(const Event& event) {
    // Pushback
    queue_mtx_.Lock();
    event_queue_.push_back(event);
    queue_mtx_.Unlock();

    //Notify(event);
}

void EventSystem::RegisterForEvent(EventListener* pListener, Event::EventType type) {
    listen_mtx_.Lock();
    listener_map_[type].push_back(pListener);
    listen_mtx_.Unlock();
}

void EventSystem::UnregisterFromEvent(const EventListener* pListener, Event::EventType type) {
    listen_mtx_.Lock();
    Listeners::iterator itr = listener_map_[type].begin();
    for(;itr != listener_map_[type].end(); itr++) {
        if(pListener == *itr) {
            listener_map_[type].erase(itr);
            break;
        }
    }
    listen_mtx_.Unlock();
}

}} //namespace
