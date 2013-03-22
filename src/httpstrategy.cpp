#include "httpstrategy.h"

HttpStrategyInterface::HttpStrategyInterface() {
    m_pCredentialsManager = NULL;
    m_pFileManager = NULL;
    m_pDelegate = NULL;
}

HttpStrategyInterface::~HttpStrategyInterface() {
    m_pCredentialsManager = NULL;
    m_pFileManager = NULL;
    if(m_pDelegate) {
        delete m_pDelegate;
        m_pDelegate = NULL;
    }
}

void HttpStrategyInterface::SetCallbackDelegate(TaskDelegate* pDel) { 
    if(m_pDelegate) {
        delete m_pDelegate;
        m_pDelegate = NULL;
    }
    m_pDelegate = pDel;
}

void HttpStrategyInterface::Callback(const int tasktype, 
                                     const int code, 
                                     const int taskstate, 
                                     const std::string& var) 
{
    if(m_pDelegate)
        m_pDelegate->Callback(tasktype, code, taskstate, var);
}




HttpStrategyContext::HttpStrategyContext(FileManager* pFileManager,
                                         CredentialsManager* pCredentialsManager)
{
    m_pFileManager = pFileManager;
    m_pCredentialsManager = pCredentialsManager;
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
