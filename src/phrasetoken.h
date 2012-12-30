
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

    void GetKey(std::string& out) { out = m_Key; }

    void SetKey(const std::string& key) { m_Key = key; }

private:
    std::string m_Key; // Key generated from passphrase

};

#endif

