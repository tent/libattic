#ifndef CONNECTIONHANDLE_H_
#define CONNECTIONHANDLE_H_
#pragma once

#include <curl/curl.h>

class ConnectionHandle
{
public:
    ConnectionHandle() 
    {
        m_pCurlHandle = NULL;
    }

    ~ConnectionHandle() 
    {
        Cleanup();
    }

    void Init() { m_pCurlHandle = curl_easy_init(); }
    void Cleanup() 
    {
        if(m_pCurlHandle)
        {
            curl_easy_cleaup(m_pCurlHandle);
            m_pCurlHandle = NULL;
        }
    }


    CURL* GetConnectionHandle() { return m_pCurlHandle; }
private:
    CURL* pCurlHandle;
}
#endif

