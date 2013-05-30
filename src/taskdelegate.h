#ifndef TASKDELEGATE_H_
#define TASKDELEGATE_H_
#pragma once

#include <string>
#include "crypto.h"

namespace attic { 

class TaskDelegate {
public:                                                         
    enum DelegateType {
        TASK=0,
        MANIFEST
    };

    TaskDelegate(DelegateType type) {
        type_ = type;
    }

    ~TaskDelegate() {}

    const std::string& GenerateIdentifier() {
        identifier_.clear();
        crypto::GenerateRandomString(identifier_);
        return identifier_;
    }

    virtual void Callback(const int type,
                          const int code,
                          const int state,
                          const std::string& var) const = 0;

    const std::string& identifier() const { return identifier_; }
    int type() { return type_; }
private:
    DelegateType type_;
    std::string identifier_;
};

}//namespace
#endif

