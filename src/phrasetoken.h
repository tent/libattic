
#ifndef PHRASETOKEN_H_
#define PHRASETOKEN_H_
#pragma once

#include <string>
#include "jsonserializable.h"

class PhraseToken : public JsonSerializable
{
public:
    PhraseToken();
    ~PhraseToken();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    int SaveToFile(const std::string& filepath);
    int LoadFromFile(const std::string& filepath);

    void GetKey(std::string& out) { out = m_Key; }
    void GetSalt(std::string& out) { out = m_Salt; }

    void SetKey(const std::string& key) { m_Key = key; }
    void SetSalt(const std::string& salt) { m_Salt = salt; }

private:
    std::string m_Key; // Key generated from passphrase
    std::string m_Salt;

};

#endif

