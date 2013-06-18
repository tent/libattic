#ifndef TASKDELEGATE_H_
#define TASKDELEGATE_H_
#pragma once

#include <string>
#include "crypto.h"

namespace attic { 

namespace cbh {
    typedef void(*DelegateCallback)(int, int, const char*);
    typedef void(*QueryCallback)(int, char**, int, int);
    typedef void(*HistoryCallback)(int, const char*, int, int);
};

class CallbackHandler;
class TaskDelegate {
public:                                                         
    enum DelegateType {
        TASK=0,
        MANIFEST,
        FILEHISTORY
    };

    TaskDelegate(DelegateType type) {
        type_ = type;
    }

    virtual ~TaskDelegate() {}

    const std::string& GenerateIdentifier() {
        identifier_.clear();
        utils::GenerateRandomString(identifier_);
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

class HistoryCallback : public TaskDelegate {
public:
    HistoryCallback(CallbackHandler* handler, cbh::HistoryCallback cb);
    ~HistoryCallback() {}

    void Callback(const int type,
                  const int code,
                  const int state,
                  const std::string& var) const;

    void Callback(const int code, 
                  const char* buffer,
                  const int stride,
                  const int total); 
private:
    cbh::HistoryCallback cb_;
    CallbackHandler* owner_;
};

class ManifestCallback : public TaskDelegate { 
public:
    ManifestCallback(CallbackHandler* handler, cbh::QueryCallback cb);
    ~ManifestCallback() {}

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

}//namespace
#endif

