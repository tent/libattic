#ifndef TASKCONTEXT_H_
#define TASKCONTEXT_H_
#pragma once

#include <map>
#include <deque>
#include <string>
#include "taskdelegate.h"

namespace attic { 

class TaskContext {
public:
    typedef std::deque<TaskContext> ContextQueue;

    TaskContext();
    ~TaskContext();

    void set_value(const std::string& key, const std::string& value);
    bool get_value(const std::string& key, std::string& out) const;

    TaskDelegate* delegate() const  { return delegate_; }
    void set_delegate(TaskDelegate* del) { delegate_ = del; }

    int type() const { return type_; }
    void set_type(int type) { type_ = type; }
private:
    typedef std::map<std::string, std::string> ValueMap;
    ValueMap value_map_;
    TaskDelegate* delegate_;
    int type_; // Corellates to Task::TaskType
};

}//namespace
#endif

