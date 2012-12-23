
#ifndef ACCESSTOKEN_H_
#define ACCESSTOKEN_H_
#pragma once

#include <string>
#include "errorcodes.h"
#include "jsonserializable.h"

class AccessToken : public JsonSerializable
{
public:
    AccessToken();
    ~AccessToken();

    // TODO :: move this to some other place, the file manager could 
    //         be in charge of saving state to disk
    ret::eCode SaveToFile(const std::string& filepath);
    ret::eCode LoadFromFile(const std::string& filepath);

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    std::string GetAccessToken() const   { return m_AccessToken; }
    std::string GetMacKey() const        { return m_MacKey; }
    std::string GetMacAlgorithm() const  { return m_MacAlgorithm; }
    std::string GetTokenType() const     { return m_TokenType; }

    void Reset()
    {
        m_AccessToken.clear();
        m_MacKey.clear();
        m_MacAlgorithm.clear();
        m_TokenType.clear();
    }

private:
    std::string m_AccessToken;
    std::string m_MacKey;
    std::string m_MacAlgorithm;
    std::string m_TokenType;
};


#endif

