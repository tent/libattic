
#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_
#pragma once

#include <list>
#include <string>
#include <curl/curl.h>

#include <osrng.h>

#include "post.h"

struct Response
{
    int code;
    std::string body;
};

class UrlParams;

class ConnectionManager
{
    ConnectionManager();
    ConnectionManager(const ConnectionManager &rhs) {}
    ~ConnectionManager();
    ConnectionManager operator=(const ConnectionManager &rhs) { return *this; }

    void EncodeAndAppendUrlParams( CURL* pCurl, 
                                   const UrlParams* pParams, 
                                   std::string &url);

    void GenerateNonce(std::string &out);
    void GenerateHmacSha256(std::string &out);

    void AddBodyToForm( const std::string &body,
                        curl_httppost **post, 
                        curl_httppost **last, 
                        curl_slist **list);

    void AddAttachmentToForm( const std::string &path, 
                              curl_httppost **post, 
                              curl_httppost **last, 
                              curl_slist **list);

    int GetResponseCode(CURL* pCurl);
public:
    void BuildAuthHeader( const std::string &url, 
                          const std::string &requestMethod, 
                          const std::string &macid, 
                          const std::string &mackey, 
                          std::string &out);

    void SignRequest( const std::string &request, 
                      const std::string &key, 
                      std::string &out);

    void Initialize();
    void Shutdown();

    // Note * the reason for the singleton, is that libcurl is not threadsafe
    // when it comes to sharing handles, at this point we don't need threads
    // passing around handles and using a multihandle arch, so abstract it out.
    // Also callbacks need to be static.
    // 
    static ConnectionManager* GetInstance();

    int HttpGet( const std::string &url, 
                 const UrlParams* pParams, 
                 Response& responseOut,
                 bool verbose = false);

    int HttpGetWithAuth( const std::string &url, 
                         const UrlParams* pParams,
                         Response& responseOut, 
                         const std::string &macalgorithm, 
                         const std::string &macid, 
                         const std::string &mackey, 
                         bool verbose = false);

    int HttpGetAttachmentWriteToFile( const std::string &url, 
                                      const UrlParams* pParams,
                                      Response& responseOut, 
                                      const std::string &filepath, 
                                      const std::string &macalgorithm, 
                                      const std::string &macid, 
                                      const std::string &mackey, 
                                      bool verbose = false);

    int HttpPost( const std::string& url, 
                  const UrlParams* pParams,
                  const std::string& body, 
                  Response& responseOut, 
                  bool verbose = false);

    int HttpDelete( const std::string &url,
                    const UrlParams* pParams,
                    Response& responseOut,
                    const std::string &macalgorithm, 
                    const std::string &macid, 
                    const std::string &mackey, 
                    bool verbose = false);

    int HttpPostWithAuth( const std::string &url, 
                          const UrlParams* pParams,
                          const std::string &body, 
                          Response& responseOut, 
                          const std::string &macalgorithm, 
                          const std::string &macid, 
                          const std::string &mackey, 
                          bool verbose = false);

    int HttpPutWithAuth( const std::string &url, 
                          const UrlParams* pParams,
                          const std::string &body, 
                          Response& responseOut, 
                          const std::string &macalgorithm, 
                          const std::string &macid, 
                          const std::string &mackey, 
                          bool verbose = false);


    int HttpMultipartPost( const std::string &url, 
                           const UrlParams* pParams,
                           const std::string &body, 
                           std::list<std::string>* filepaths, 
                           Response& responseOut, 
                           const std::string &macalgorithm, 
                           const std::string &macid, 
                           const std::string &mackey, 
                           bool verbose = false);

    int HttpMultipartPut( const std::string &url, 
                          const UrlParams* pParams,
                          const std::string &body, 
                          std::list<std::string>* filepaths, 
                          Response& responseOut, 
                          const std::string &macalgorithm, 
                          const std::string &macid, 
                          const std::string &mackey, 
                          bool verbose = false);
    
private:
    CryptoPP::AutoSeededRandomPool  m_Rnd; // Random pool used for nonce(iv) generation
    static ConnectionManager *m_pInstance;
};

#endif

