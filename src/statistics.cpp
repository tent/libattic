#include "statistics.h"

#include <iostream>
#include <sstream>

namespace attic { 

Statistics* Statistics::instance_ = 0;

void Statistics::Shutdown() {
    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }
}

Statistics* Statistics::instance() {
    if(!instance_) 
        instance_ = new Statistics();
    return instance_;
}

void Statistics::IncrementValue(const std::string& key) {
    Lock();
    StatsMap::iterator itr = stats_map_.find(key);
    if(itr != stats_map_.end())
        itr->second++;
    else
        stats_map_[key] = 1;
    Unlock();
}

void Statistics::DecrementValue(const std::string& key) {
    Lock();
    StatsMap::iterator itr = stats_map_.find(key);
    if(itr != stats_map_.end())
        if(itr->second > 0)
            itr->second--;
    else
        stats_map_[key] = 0;
    Unlock();
}

void Statistics::PrintStats() {
    std::ostringstream oss;
    Lock();
    StatsMap::iterator itr = stats_map_.begin();
    for(;itr != stats_map_.end(); itr++) {
        oss << itr->first << " : " << itr->second << std::endl;
    }
    Unlock();
    std::cout<< oss << std::endl;
}

} // namespace
