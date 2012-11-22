

#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_
#pragma once

#include <string>
#include <curl/curl.h>

#include <osrng.h>

class ConnectionManager
{
    ConnectionManager();
    ConnectionManager(const ConnectionManager &rhs) {}
    ~ConnectionManager();
    ConnectionManager operator=(const ConnectionManager &rhs) { return *this; }

    void GenerateNonce(std::string &out);
    void GenerateHmacSha256(std::string &out);


public:

    void BuildAuthHeader(const std::string &url, const std::string &requestMethod, const std::string &szMacID, const std::string &szMacKey, std::string &out);
    void SignRequest(const std::string &szRequest, const std::string &szKey, std::string &out);
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

    void HttpPostWithAuth(const std::string &url, const std::string &body, std::string &responseOut, const std::string &szMacAlgorithm, const std::string &szMacID, const std::string &szMacKey, bool versbose = false);

private:
    CryptoPP::AutoSeededRandomPool  m_Rnd; // Random pool used for nonce(iv) generation
    static ConnectionManager *m_pInstance;

    CURL* m_pCurl;  // Curl instance

};

#endif

