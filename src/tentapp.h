#ifndef TENTAPP_H_
#define TENTAPP_H_
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "errorcodes.h"
#include "jsonserializable.h"
#include "accesstoken.h"

namespace attic {

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
    const std::string& hawk_algorithm() const    { return hawk_algorithm_; }
    const std::string& hawk_key_id() const       { return hawk_key_id_; }
    const std::string& hawk_key() const          { return hawk_key_; }
    const std::string& redirect_uri() const      { return redirect_uri_; }
    ScopeVec* scopes()                           { return &scopes_; }
    AuthVec* authorizations()                    { return &authorizations_;}
    
    void set_app_id(const std::string &id)                      { app_id_ = id; }
    void set_app_name(const std::string &name)                  { app_name_ = name; }
    void set_app_description(const std::string &description)    { app_description_ = description; }
    void set_app_url(const std::string &url)                    { app_url_ = url; }
    void set_app_icon(const std::string &icon)                  { app_icon_ = icon; }
    void set_hawk_algorithm(const std::string &alg)              { hawk_algorithm_ = alg; }
    void set_hawk_key_id(const std::string &id)                  { hawk_key_id_ = id; }
    void set_hawk_key(const std::string &key)                    { hawk_key_ = key; }
    void set_scopes(const std::vector<std::string>& scopes)     { scopes_ = scopes; }
    void set_redirect_uri(const std::string& uri)               { redirect_uri_ = uri; }
    void set_authorizations(const std::vector<std::string>& auth)   { authorizations_ = auth; }

    void PushBackScope(const std::string &szScope)          { scopes_.push_back(szScope); }
    void PushBackAuthorization(const std::string &szAuth)   { authorizations_.push_back(szAuth); }
private:
    std::vector<std::string> scopes_;
    std::vector<std::string> authorizations_;

    std::string app_id_;
    std::string app_name_;
    std::string app_description_;
    std::string app_url_;
    std::string app_icon_;
    std::string hawk_algorithm_;
    std::string hawk_key_id_;
    std::string hawk_key_;
    std::string redirect_uri_;
};

}//namespace
#endif

