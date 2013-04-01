#ifndef HTTPSTRATEGY_H_
#define HTTPSTRATGEY_H_
#pragma once

#include <map>
#include <list>
#include <string>

#include "response.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "taskdelegate.h"

namespace attic {

class HttpTentContext;

class HttpStrategyInterface {
    friend class HttpStrategyContext;
    typedef std::map<std::string, std::string> ConfigMap;
protected:
    std::string GetConfigValue(const std::string& key) { return config_map_[key]; }
public:
    HttpStrategyInterface();
    ~HttpStrategyInterface();

    virtual int Execute(FileManager* pFileManager,
                        CredentialsManager* pCredentialsManager,
                        Response& out)=0;

    // Takes ownership of the delegate.
    // Will delete previous delegate if set more than once
    void set_task_delegate(TaskDelegate* del);

    void Callback(const int tasktype, 
                  const int code, 
                  const int taskstate, 
                  const std::string& var);
protected:
    ConfigMap               config_map_;
    CredentialsManager*     credentials_manager_;
    FileManager*            file_manager_;

    AccessToken             access_token_;
    std::string             post_path_;
    std::string             posts_feed_;

    TaskDelegate*           task_delegate_;
};

class HttpStrategyContext {
    typedef std::list<HttpStrategyInterface*> StrategyList;

    int Execute(HttpStrategyInterface* s, Response& out);
public:
    typedef std::map<std::string, std::string> ConfigMap;

    HttpStrategyContext(FileManager* pFileManager,
                        CredentialsManager* pCredentialsManager);

    ~HttpStrategyContext();

    void PushBack(HttpStrategyInterface* pStrat);

    int ExecuteAll(); // By default will stop at first failed execution
    int Step(Response& out);
    void ResetPosition();

    void SetConfigValue(const std::string& key, const std::string& value) {
        config_map_[key] = value;
    }

    std::string* operator[](const std::string& key) {
        return &config_map_[key];
    }

private:
    CredentialsManager*     credentials_manager_;
    FileManager*            file_manager_;

    ConfigMap       config_map_;
    StrategyList    strategies_;
    StrategyList::iterator strategy_itr_;
};

}//namespace
#endif

