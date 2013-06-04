#ifndef ACCESSTOKEN_H_
#define ACCESSTOKEN_H_
#pragma once

#include <string>
#include <stdlib.h> //strtol
#include "errorcodes.h"
#include "jsonserializable.h"

namespace attic { 
class AccessToken : public JsonSerializable {
public:
    AccessToken();
    ~AccessToken();

    // TODO :: move this to some other place, the file manager could 
    //         be in charge of saving state to disk
    ret::eCode SaveToFile(const std::string& filepath);
    ret::eCode LoadFromFile(const std::string& filepath);

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    const std::string& access_token() const      { return access_token_; }
    const std::string& hawk_key() const          { return hawk_key_; }
    const std::string& hawk_algorithm() const    { return hawk_algorithm_; }
    const std::string& token_type() const        { return token_type_; }
    const std::string& app_id() const            { return app_id_; } 
    long int time_offset() const                 { return strtol(time_offset_.c_str(), NULL, 0); }

    void set_access_token(const std::string& token)         { access_token_ = token; }
    void set_hawk_key(const std::string& key)               { hawk_key_ = key; } 
    void set_hawk_algorithm(const std::string& algorithm)   { hawk_algorithm_ = algorithm; } 
    void set_token_type(const std::string& token_type)      { token_type_ = token_type; }
    void set_app_id(const std::string& app_id)              { app_id_ = app_id; } 
    void set_time_offset(const std::string& ts)             { time_offset_ = ts; }

    void Reset();
private:
    std::string access_token_;
    std::string hawk_key_;
    std::string hawk_algorithm_;
    std::string token_type_;
    std::string app_id_;
    std::string time_offset_;
};


}//namespace
#endif

