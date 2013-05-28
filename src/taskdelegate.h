#ifndef TASKDELEGATE_H_
#define TASKDELEGATE_H_
#pragma once

#include <string>
#include "crypto.h"

namespace attic { 

class TaskDelegate {
public:                                                         
    TaskDelegate() {}
    ~TaskDelegate() {}

    void Initialize() {
        crypto::GenerateRandomString(identifier_);
    }

                                                                                
    virtual void Callback(const int type,
                          const int code,
                          const int state,
                          const std::string& var) const = 0;

    const std::string& identifier() const { return identifier_; }
private:
    std::string identifier_;

};

}//namespace
#endif

