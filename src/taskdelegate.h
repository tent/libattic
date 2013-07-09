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
    // error code, payload, error string
    typedef void(*RequestCallback)(int, const char*, const char*); 
};

class CallbackHandler;

class TaskDelegate {
public:                                                         
    enum DelegateType {
        TASK=0,
        MANIFEST,
        FILEHISTORY,
        REQUEST
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

class HistoryDelegate : public TaskDelegate {
public:
    HistoryDelegate(CallbackHandler* handler, cbh::HistoryCallback cb);
    ~HistoryDelegate() {}

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

class ManifestDelegate : public TaskDelegate { 
public:
    ManifestDelegate(CallbackHandler* handler, cbh::QueryCallback cb);
    ~ManifestDelegate() {}

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

class RequestDelegate : public TaskDelegate { 
public:
     RequestDelegate(CallbackHandler* handler, cbh::RequestCallback cb);
    ~RequestDelegate() {}

    void Callback(const int type,
                  const int code,
                  const int state,
                  const std::string& var) const;

    void Callback(const int code, 
                  const char* payload,
                  const char* error);
private:
    cbh::RequestCallback cb_;
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

