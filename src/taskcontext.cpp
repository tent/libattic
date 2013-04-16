#include "taskcontext.h"

namespace attic { 

TaskContext::TaskContext() {}
TaskContext::~TaskContext() {}

void TaskContext::set_value(const std::string& key, const std::string& value) {
    value_map_[key] = value;
}

bool TaskContext::get_value(const std::string& key, std::string& out) {
    ValueMap::iterator itr = value_map_.find(key);
    if(itr != value_map_.end()) { 
        out = itr->second;
        return true;
    }
    return false;
}

}//namespace
