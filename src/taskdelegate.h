#ifndef TASKDELEGATE_H_
#define TASKDELEGATE_H_
#pragma once

#include <string>

namespace attic { 

class TaskDelegate {
public:                                                         
    TaskDelegate() {}
    ~TaskDelegate() {}
                                                                                
    virtual void Callback(const int type,
                          const int code,
                          const int state,
                          const std::string& var) const = 0;
};

}//namespace
#endif

