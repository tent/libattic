
#ifndef TENTAPP_H_
#define TENTAPP_H_
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "jsonserializable.h"

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

class TentApp : public JsonSerializable
{
    void SerializeVector(Json::Value &val, std::vector<std::string> &vec);
    void DeserializeIntoVector(Json::Value &val, std::vector<std::string> &vec);
public:
    TentApp();
    ~TentApp();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void RegisterApp();
    void RequestAuthorization(std::string& szAppID, RedirectCode& tRedirectCode);
    std::string RequestAuthorizationURL();
    std::auto_ptr<AccessToken> RequestUserAuthorizationDetails(std::string szCode);

    std::string GetAppID() const            { return m_AppID; }
    std::string GetAppName() const          { return m_AppName; }
    std::string GetAppDescription() const   { return m_AppDescription; }
    std::string GetAppHomepageURL() const   { return m_AppHomepageURL; }
    std::string GetAppIcon() const          { return m_AppIcon; }
    std::string GetMacAlgorithm() const     { return m_MacAlgorithm; }
    std::string GetMacKeyID() const         { return m_MacKeyID; }
    std::string GetMacKey() const           { return m_MacKey; }
    
    std::vector<std::string>* GetScopes()           { return &m_Scopes; }
    std::vector<std::string>* GetRedirectURIs()     { return &m_RedirectURIs;}
    std::vector<std::string>* GetAuthorizations()   { return &m_Authorizations;}
    
    void SetAppID(const std::string &szID)                      { m_AppID = szID; }
    void SetAppName(const std::string &szName)                  { m_AppName = szName; }
    void SetAppDescription(const std::string &szDescription)    { m_AppDescription = szDescription; }
    void SetAppHomepageURL(const std::string &szURL)            { m_AppHomepageURL = szURL; }
    void SetAppIcon(const std::string &szIcon)                  { m_AppIcon = szIcon; }
    void SetMacAlgorithm(const std::string &szAlg)              { m_MacAlgorithm = szAlg; }
    void SetMacKeyID(const std::string &szID)                   { m_MacKeyID = szID; }
    void SetMacKey(const std::string &szKey)                    { m_MacKey = szKey; }

    void SetScope(const std::string &szScope)           { m_Scopes.push_back(szScope); }
    void SetRedirectURI(const std::string &szURI)       { m_RedirectURIs.push_back(szURI); }
    void SetAuthorization(const std::string &szAuth)    { m_Authorizations.push_back(szAuth); }

private:
    std::vector<std::string> m_Scopes;
    std::vector<std::string> m_RedirectURIs;
    std::vector<std::string> m_Authorizations;

    std::string m_AppID;
    std::string m_AppName;
    std::string m_AppDescription;
    std::string m_AppHomepageURL;
    std::string m_AppIcon;
    std::string m_MacAlgorithm;
    std::string m_MacKeyID;
    std::string m_MacKey;
};

#endif

