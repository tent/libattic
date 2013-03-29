#ifndef PHRASETOKEN_H_
#define PHRASETOKEN_H_
#pragma once

#include <string>
#include "jsonserializable.h"

class PhraseToken : public JsonSerializable {
public:
    PhraseToken() {}
    ~PhraseToken() {}

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    int SaveToFile(const std::string& filepath);
    int LoadFromFile(const std::string& filepath);
    
    bool IsPhraseKeyEmpty() { return phrase_key_.empty(); }
    bool IsSaltEmpty() { return salt_.empty(); }

    const std::string& dirty_key() const    { return dirty_key_; }
    const std::string& phrase_key() const   { return phrase_key_; }
    const std::string& salt() const         { return salt_; }
    const std::string& iv() const           { return iv_; }

    void set_dirty_key(const std::string& key)  { dirty_key_ = key; }
    void set_phrase_key(const std::string& key) { phrase_key_ = key; }
    void set_salt(const std::string& salt)      { salt_ = salt; }
    void set_iv(const std::string& iv)          { iv_ = iv; }

private:
    std::string dirty_key_; // Key generated from passphrase, encrypted, with sentinel values
    std::string phrase_key_; // Key generated from passphrase, unencrypted
    std::string salt_;
    std::string iv_; // Iv used to encrypt master key
};

#endif

