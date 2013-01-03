
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
    
    bool IsKeyEmpty() { return m_Key.empty(); }
    bool IsSaltEmpty() { return m_Salt.empty(); }

    void GetDirtyKey(std::string& out) const { out = m_DirtyKey; }
    void GetKey(std::string& out) const { out = m_Key; }
    void GetSalt(std::string& out) const { out = m_Salt; }
    void GetIv(std::string& out) const { out = m_Iv; }

    void SetDirtyKey(const std::string& key) { m_DirtyKey = key; }
    void SetKey(const std::string& key) { m_Key = key; }
    void SetSalt(const std::string& salt) { m_Salt = salt; }
    void SetIv(const std::string& iv) { m_Iv = iv; }

private:
    std::string m_DirtyKey; // Key encrypted, with sentinel values

    std::string m_Key; // Key generated from passphrase, unencrypted
    std::string m_Salt;
    std::string m_Iv; // Iv used to encrypt master key

};

#endif

