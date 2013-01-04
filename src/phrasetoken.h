
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
    
    bool IsPhraseKeyEmpty() { return m_PhraseKey.empty(); }
    bool IsSaltEmpty() { return m_Salt.empty(); }

    void GetDirtyKey(std::string& out) const { out = m_DirtyKey; }
    void GetPhraseKey(std::string& out) const { out = m_PhraseKey; }
    void GetSalt(std::string& out) const { out = m_Salt; }
    void GetIv(std::string& out) const { out = m_Iv; }

    void SetDirtyKey(const std::string& key) { m_DirtyKey = key; }
    void SetPhraseKey(const std::string& key) { m_PhraseKey = key; }
    void SetSalt(const std::string& salt) { m_Salt = salt; }
    void SetIv(const std::string& iv) { m_Iv = iv; }

private:
    std::string m_DirtyKey; // Key generated from passphrase, encrypted, with sentinel values
    std::string m_PhraseKey; // Key generated from passphrase, unencrypted
    std::string m_Salt;
    std::string m_Iv; // Iv used to encrypt master key

};

#endif

