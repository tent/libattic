#ifndef TENTAPP_H_
#define TENTAPP_H_
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "errorcodes.h"
#include "jsonserializable.h"
#include "accesstoken.h"

class RedirectCode : public JsonSerializable
{
public:
    RedirectCode();
    ~RedirectCode();
    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    std::string GetCode() { return m_Code; }
    std::string GetTokenType() { return m_TokenType; }

    void SetCode(const std::string& code) { m_Code = code; }
    void SetTokenType (const std::string& type) { m_TokenType = type; }
private:
    std::string m_Code;
    std::string m_TokenType;
};

class TentApp : public JsonSerializable {
public:
    TentApp() {}
    ~TentApp() {}

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    ret::eCode SaveToFile(const std::string& filepath);
    ret::eCode LoadFromFile(const std::string& filepath);

    const std::string& GetAppID() const            { return m_AppID; }
    const std::string& GetAppName() const          { return m_AppName; }
    const std::string& GetAppDescription() const   { return m_AppDescription; }
    const std::string& GetAppURL() const           { return m_AppURL; }
    const std::string& GetAppIcon() const          { return m_AppIcon; }
    const std::string& GetMacAlgorithm() const     { return m_MacAlgorithm; }
    const std::string& GetMacKeyID() const         { return m_MacKeyID; }
    const std::string& GetMacKey() const           { return m_MacKey; }
    
    typedef std::vector<std::string> ScopeVec;
    typedef std::vector<std::string> RedirectVec;
    typedef std::vector<std::string> AuthVec;

    ScopeVec* GetScopes()           { return &m_Scopes; }
    RedirectVec* GetRedirectURIs()  { return &m_RedirectURIs;}
    AuthVec* GetAuthorizations()    { return &m_Authorizations;}
    
    void SetAppID(const std::string &szID)                      { m_AppID = szID; }
    void SetAppName(const std::string &szName)                  { m_AppName = szName; }
    void SetAppDescription(const std::string &szDescription)    { m_AppDescription = szDescription; }
    void SetAppURL(const std::string &szURL)                    { m_AppURL = szURL; }
    void SetAppIcon(const std::string &szIcon)                  { m_AppIcon = szIcon; }
    void SetMacAlgorithm(const std::string &szAlg)              { m_MacAlgorithm = szAlg; }
    void SetMacKeyID(const std::string &szID)                   { m_MacKeyID = szID; }
    void SetMacKey(const std::string &szKey)                    { m_MacKey = szKey; }

    void SetScopes(const std::vector<std::string>& scopes) { m_Scopes = scopes; }
    void PushBackScope(const std::string &szScope)      { m_Scopes.push_back(szScope); }
    void SetRedirectUris(const std::vector<std::string>& uris) { m_RedirectURIs = uris; }
    void PushBackRedirectUri(const std::string &szURI)  { m_RedirectURIs.push_back(szURI); }
    void SetAuthorizations(const std::vector<std::string>& auth) { m_Authorizations = auth; }
    void PushBackAuthorization(const std::string &szAuth) { m_Authorizations.push_back(szAuth); }

private:
    std::vector<std::string> m_Scopes;
    std::vector<std::string> m_RedirectURIs;
    std::vector<std::string> m_Authorizations;

    std::string m_AppID;
    std::string m_AppName;
    std::string m_AppDescription;
    std::string m_AppURL;
    std::string m_AppIcon;
    std::string m_MacAlgorithm;
    std::string m_MacKeyID;
    std::string m_MacKey;
};

#endif

