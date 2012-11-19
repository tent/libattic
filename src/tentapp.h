
#ifndef TENTAPP_H_
#define TENTAPP_H_
#pragma once

#include <string>
#include <vector>
#include <memory>

struct RedirectCode
{
    std::string Code;
    std::string TokenType;
};

struct AccessToken
{
    std::string AccessToken;
    std::string MacKey;
    std::string MacAlgorithm;
    std::string TokenType;
};

class TentApp
{
public:
    TentApp();
    ~TentApp();

    void RegisterApp();
    void RequestAuthorization(std::string& szAppID, RedirectCode& tRedirectCode);
    std::string RequestAuthorizationURL();
    std::auto_ptr<AccessToken> RequestUserAuthorizationDetails(std::string szCode);

private:
    std::vector<std::string> m_Scopes;
    std::vector<std::string> m_RedirectURIs;
    std::vector<std::string> m_Authorizations;

    std::string m_Id;
    std::string m_AppName;
    std::string m_AppDescription;
    std::string m_AppHomepageURL;
    std::string m_AppIcon;
    std::string m_MacAlgorithm;
    std::string m_MacKeyID;
    std::string MacKey;

};

#endif

