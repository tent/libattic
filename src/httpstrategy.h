#ifndef HTTPSTRATEGY_H_
#define HTTPSTRATGEY_H_
#pragma once

#include <map>
#include <list>
#include <string>

#include "response.h"
#include "filemanager.h"
#include "credentialsmanager.h"

class HttpTentContext;

class HttpStrategyInterface {
    friend class HttpStrategyContext;
    typedef std::map<std::string, std::string> ConfigMap;
public:
    virtual void Execute(FileManager* pFileManager,
                         CredentialsManager* pCredentialsManager,
                         const std::string& entityApiRoot, 
                         const std::string& filepath, 
                         Response& out)=0;
protected:
    ConfigMap m_ConfigMap;
};

class HttpStrategyContext {
    typedef std::list<HttpStrategyInterface*> StrategyList;

    void Execute(HttpStrategyInterface* s, Response& out);
public:
    typedef std::map<std::string, std::string> ConfigMap;

    HttpStrategyContext(FileManager* pFileManager,
                        CredentialsManager* pCredentialsManager,
                        const std::string& entityApiRoot, 
                        const std::string& filepath);

    ~HttpStrategyContext();

    void PushBack(HttpStrategyInterface* pStrat);

    void ExecuteAll();
    void Step(Response& out);
    void ResetPosition();

    std::string* operator[](const std::string& key) {
        return &m_ConfigMap[key];
    }

private:
    CredentialsManager*     m_pCredentialsManager;
    FileManager*            m_pFileManager;
    std::string             m_EntityApiRoot;
    std::string             m_Filepath;

    ConfigMap   m_ConfigMap;
    StrategyList m_Strategies;
    StrategyList::iterator m_Itr;
};

#endif

