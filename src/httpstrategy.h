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

class HttpTentContext;

class HttpStrategyInterface {
    friend class HttpStrategyContext;
    typedef std::map<std::string, std::string> ConfigMap;
protected:
    std::string GetConfigValue(const std::string& key) { return m_ConfigMap[key]; }
public:
    HttpStrategyInterface();
    ~HttpStrategyInterface();

    virtual int Execute(FileManager* pFileManager,
                        CredentialsManager* pCredentialsManager,
                        Response& out)=0;

    // Takes ownership of the delegate.
    // Will delete previous delegate if set more than once
    void SetCallbackDelegate(TaskDelegate* pDel);

    void Callback(const int tasktype, 
                  const int code, 
                  const int taskstate, 
                  const std::string& var);
protected:
    ConfigMap m_ConfigMap;
    CredentialsManager*     m_pCredentialsManager;
    FileManager*            m_pFileManager;

    AccessToken             m_At;
    std::string             m_entityApiRoot;

    TaskDelegate*           m_pDelegate;
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
        m_ConfigMap[key] = value;
    }

    std::string* operator[](const std::string& key) {
        return &m_ConfigMap[key];
    }

private:
    CredentialsManager*     m_pCredentialsManager;
    FileManager*            m_pFileManager;

    ConfigMap   m_ConfigMap;
    StrategyList m_Strategies;
    StrategyList::iterator m_Itr;
};

#endif

