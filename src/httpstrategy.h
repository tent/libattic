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
    bool  HasConfigValue(const std::string& key) { 
        if(config_map_.find(key) != config_map_.end())
            return true;
        return false;
    }

    std::string GetAllConfigs() {
        std::string buf;
        ConfigMap::iterator itr = config_map_.begin();
        for(;itr != config_map_.end(); itr++) {
            buf += "\t" + itr->first + "  |  " + itr->second + "\n";
        }
        return buf;
    }

    int InitInstance(FileManager* fm,
                     CredentialsManager* cm);
public:
    HttpStrategyInterface();
    ~HttpStrategyInterface();

    virtual int Execute(FileManager* fm,
                        CredentialsManager* cm)=0;

    // Takes ownership of the delegate.
    // Will delete previous delegate if set more than once
    void set_task_delegate(TaskDelegate* del);

    void Callback(const int tasktype, 
                  const int code, 
                  const int taskstate, 
                  const std::string& var);
protected:
    bool GetMasterKey(std::string& out) {
        MasterKey mKey;
        credentials_manager_->GetMasterKeyCopy(mKey);
        mKey.GetMasterKey(out);
        if(out.size())
            return true;
        return false;
    }

    std::string GetMasterKey() {
        std::string master_key;
        GetMasterKey(master_key);
        return master_key;
    }

    bool ValidMasterKey() {
        std::string mk;
        return GetMasterKey(mk);
    }

protected:
    ConfigMap               config_map_;
    CredentialsManager*     credentials_manager_;
    FileManager*            file_manager_;

    AccessToken             access_token_;
    std::string             post_path_;
    std::string             posts_feed_;
    std::string             post_attachment_;
    std::string             entity_;

    TaskDelegate*           task_delegate_;
};

class HttpStrategyContext {
    typedef std::list<HttpStrategyInterface*> StrategyList;

    int Execute(HttpStrategyInterface* s);
public:
    typedef std::map<std::string, std::string> ConfigMap;

    HttpStrategyContext(FileManager* fm,
                        CredentialsManager* cm);

    ~HttpStrategyContext();

    void PushBack(HttpStrategyInterface* pStrat);

    int ExecuteAll(); // By default will stop at first failed execution
    int Step();
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

