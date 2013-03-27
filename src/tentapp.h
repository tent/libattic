#ifndef TENTAPP_H_
#define TENTAPP_H_
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "errorcodes.h"
#include "jsonserializable.h"
#include "accesstoken.h"

class RedirectCode : public JsonSerializable {
public:
    RedirectCode() {}
    ~RedirectCode() {}
    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    const std::string& code() const         { return code_; }
    const std::string& token_type() const   { return token_type_; }

    void set_code(const std::string& code) { code_ = code; }
    void set_token_type(const std::string& type) { token_type_ = type; }
private:
    std::string code_;
    std::string token_type_;
};

class TentApp : public JsonSerializable {
public:
    typedef std::vector<std::string> ScopeVec;
    typedef std::vector<std::string> RedirectVec;
    typedef std::vector<std::string> AuthVec;

    TentApp() {}
    ~TentApp() {}

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    ret::eCode SaveToFile(const std::string& filepath);
    ret::eCode LoadFromFile(const std::string& filepath);

    const std::string& app_id() const            { return app_id_; }
    const std::string& app_name() const          { return app_name_; }
    const std::string& app_description() const   { return app_description_; }
    const std::string& app_url() const           { return app_url_; }
    const std::string& app_icon() const          { return app_icon_; }
    const std::string& mac_algorithm() const     { return mac_algorithm_; }
    const std::string& mac_key_id() const        { return mac_key_id_; }
    const std::string& mac_key() const           { return mac_key_; }
    ScopeVec* scopes()                           { return &scopes_; }
    RedirectVec* redirect_uris()                 { return &redirect_uris_;}
    AuthVec* authorizations()                    { return &authorizations_;}
    
    void set_app_id(const std::string &id)                      { app_id_ = id; }
    void set_app_name(const std::string &name)                  { app_name_ = name; }
    void set_app_description(const std::string &description)    { app_description_ = description; }
    void set_app_url(const std::string &url)                    { app_url_ = url; }
    void set_app_icon(const std::string &icon)                  { app_icon_ = icon; }
    void set_mac_algorithm(const std::string &alg)              { mac_algorithm_ = alg; }
    void set_mac_key_id(const std::string &id)                  { mac_key_id_ = id; }
    void set_mac_key(const std::string &key)                    { mac_key_ = key; }
    void set_scopes(const std::vector<std::string>& scopes)         { scopes_ = scopes; }
    void set_redirect_uris(const std::vector<std::string>& uris)    { redirect_uris_ = uris; }
    void set_authorizations(const std::vector<std::string>& auth)   { authorizations_ = auth; }

    void PushBackScope(const std::string &szScope)          { scopes_.push_back(szScope); }
    void PushBackRedirectUri(const std::string &szURI)      { redirect_uris_.push_back(szURI); }
    void PushBackAuthorization(const std::string &szAuth)   { authorizations_.push_back(szAuth); }
private:
    std::vector<std::string> scopes_;
    std::vector<std::string> redirect_uris_;
    std::vector<std::string> authorizations_;

    std::string app_id_;
    std::string app_name_;
    std::string app_description_;
    std::string app_url_;
    std::string app_icon_;
    std::string mac_algorithm_;
    std::string mac_key_id_;
    std::string mac_key_;
};

#endif

