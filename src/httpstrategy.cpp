#include "httpstrategy.h"

HttpStrategyContext::HttpStrategyContext(FileManager* pFileManager,
                                         CredentialsManager* pCredentialsManager,
                                         const std::string& entityApiRoot, 
                                         const std::string& filepath)
{
    m_pFileManager = pFileManager;
    m_pCredentialsManager = pCredentialsManager;
    m_EntityApiRoot = entityApiRoot;
    m_Filepath = filepath;
    m_Itr = m_Strategies.begin();
}

HttpStrategyContext::~HttpStrategyContext(){
    m_pFileManager = NULL;
    m_pCredentialsManager = NULL;
    m_Strategies.clear();
}

void HttpStrategyContext::PushBack(HttpStrategyInterface* pStrat) { 
    m_Strategies.push_back(pStrat);
}

void HttpStrategyContext::Execute(HttpStrategyInterface* s, Response& out) {
    if(s) {
        // Copy context config
        s->m_ConfigMap = m_ConfigMap;
        // Execute
        s->Execute(m_pFileManager,
                   m_pCredentialsManager,
                   m_EntityApiRoot,
                   m_Filepath,
                   out);
    }

}
void HttpStrategyContext::ExecuteAll() {
    StrategyList::iterator itr = m_Strategies.begin();
    Response resp;
    for(;itr != m_Strategies.end(); itr++) {
        Execute(*itr, resp);
    }

}

void HttpStrategyContext::Step(Response& out) {
    if(m_Itr != m_Strategies.end()) {
        Execute(*m_Itr, out);
        m_Itr++;
    }
}

void HttpStrategyContext::ResetPosition() {
    m_Itr = m_Strategies.begin();
}
