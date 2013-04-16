#ifndef TASKCONTEXT_H_
#define TASKCONTEXT_H_
#pragma once

#include <map>
#include <string>

namespace attic { 

class TaskContext {
public:
    TaskContext();
    ~TaskContext();

    void set_value(const std::string& key, const std::string& value);
    bool get_value(const std::string& key, std::string& out) const;
private:
    typedef std::map<std::string, std::string> ValueMap;
    ValueMap value_map_;
};

}//namespace
#endif

