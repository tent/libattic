

#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_
#pragma once

#include <string>
#include <curl/curl.h>

class ConnectionManager
{
    ConnectionManager();
    ConnectionManager(const ConnectionManager &rhs) {}
    ~ConnectionManager();
    ConnectionManager operator=(const ConnectionManager &rhs) { return *this; }

public:
    void Initialize();
    void Shutdown();

    // Note * the reason for the singleton, is that libcurl is not threadsafe
    // when it comes to sharing handles, at this point we don't need threads
    // passing around handles and using a multihandle arch, so abstract it out.
    // Also callbacks need to be static.
    // 
    static ConnectionManager* GetInstance();

    std::string HttpGet(std::string &url);  
    void HttpPost(const std::string &url, const std::string &body, std::string &responseOut, bool versbose = false);


private:
    static ConnectionManager *m_pInstance;

    CURL* m_pCurl;  // Curl instance

};

#endif

