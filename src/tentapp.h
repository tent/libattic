
#ifndef TENTAPP_H_
#define TENTAPP_H_
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "errorcodes.h"
#include "jsonserializable.h"

class AccessToken : public JsonSerializable
{
public:
    AccessToken();
    ~AccessToken();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);


    std::string GetAccessToken() { return m_AccessToken; }
    std::string GetMacKey() { return m_MacKey; }
    std::string GetMacAlgorithm() { return m_MacAlgorithm; }
    std::string GetTokenType() { return m_TokenType; }

private:
    std::string m_AccessToken;
    std::string m_MacKey;
    std::string m_MacAlgorithm;
    std::string m_TokenType;
};

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
class TentApp : public JsonSerializable
{
    public:
    TentApp();
    ~TentApp();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    // TODO :: move this to some other place, the file manager could 
    //         be in charge of saving state to disk
    ret::eCode SaveToFile(const std::string& szFilePath);
    ret::eCode LoadFromFile(const std::string& szFilePath);

    std::string GetAppID() const            { return m_AppID; }
    std::string GetAppName() const          { return m_AppName; }
    std::string GetAppDescription() const   { return m_AppDescription; }
    std::string GetAppURL() const           { return m_AppURL; }
    std::string GetAppIcon() const          { return m_AppIcon; }
    std::string GetMacAlgorithm() const     { return m_MacAlgorithm; }
    std::string GetMacKeyID() const         { return m_MacKeyID; }
    std::string GetMacKey() const           { return m_MacKey; }
    
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
    std::string m_AppURL;
    std::string m_AppIcon;
    std::string m_MacAlgorithm;
    std::string m_MacKeyID;
    std::string m_MacKey;
};

#endif

