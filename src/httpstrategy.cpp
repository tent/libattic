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

int HttpStrategyContext::Execute(HttpStrategyInterface* s, Response& out) {
    if(s) {
        // Copy context config
        s->m_ConfigMap = m_ConfigMap;
        // Execute
        return s->Execute(m_pFileManager,
                          m_pCredentialsManager,
                          m_EntityApiRoot,
                          m_Filepath,
                          out);
    }
    return ret::A_FAIL_INVALID_PTR;
}

int HttpStrategyContext::ExecuteAll() {
    // Check config for anything interesting
    StrategyList::iterator itr = m_Strategies.begin();
    Response resp;
    int status = ret::A_OK;
    for(;itr != m_Strategies.end(); itr++) {
        status = Execute(*itr, resp);
        if(status != ret::A_OK)
            break;
    }
    return status;
}

int HttpStrategyContext::Step(Response& out) {
    int status = ret::A_OK;
    if(m_Itr != m_Strategies.end()) {
        status = Execute(*m_Itr, out);
        m_Itr++;
    }
    return status;
}

void HttpStrategyContext::ResetPosition() {
    m_Itr = m_Strategies.begin();
}
