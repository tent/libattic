#ifndef CONNECTIONHANDLER_H_
#define CONNECTIONHANDLER_H_
#pragma once

#include <string>

#include "netlib.h"
#include "urlparams.h"
#include "accesstoken.h"
#include "response.h"
#include "connectionmanager.h"

namespace attic {

class ConnectionHandler {
    int HttpRequest(const std::string& url, 
                    boost::asio::streambuf& request,
                    Response& out);
public:
    ConnectionHandler();
    ~ConnectionHandler();

    void SetEntityUrlOnManager(const std::string& entity_url);

    int HttpPost(const std::string& url, 
                 const std::string& post_type,
                 const UrlParams* pParams,
                 const std::string& requestbody,
                 const AccessToken* at, 
                 Response& out);

    int HttpPut(const std::string& url, 
                   const std::string& post_type,
                   const UrlParams* pParams,
                   const std::string& requestbody,
                   const AccessToken* at, 
                   Response& out);

    int HttpGet(const std::string& url, 
                const UrlParams* pParams,
                const AccessToken* at, 
                Response& out);

    int HttpHead(const std::string& url, 
                 const UrlParams* pParams,
                 const AccessToken* at, 
                 Response& out);

    int HttpDelete(const std::string& url, 
                   const UrlParams* pParams,
                   const AccessToken* at, 
                   Response& out);
private:
    ConnectionManager* manager_instance_;
};

} //namespace
#endif

