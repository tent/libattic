#ifndef ATTICPOST_H_
#define ATTICPOST_H_
#pragma once

#include <string>
#include "post.h"

class AtticPost : public Post { 
public:
    AtticPost();
    ~AtticPost();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    const std::string& salt() const             { return salt_; }
    const std::string& master_key() const       { return master_key_; }

    const std::string& recovery_salt() const            { return recovery_salt_; }
    const std::string& recovery_master_key() const      { return recovery_master_key_; }
    
    const std::string& question_salt() const            { return question_salt_; }
    const std::string& question_master_key() const      { return question_master_key_; }

    const std::string& question_one() const     { return question_one_; }
    const std::string& question_two() const     { return question_two_; }
    const std::string& question_three() const   { return question_three_; }

    void set_salt(const std::string& salt)          { salt_ = salt; }
    void set_master_key(const std::string& key)     { master_key_ = key; }

    void set_recovery_salt(const std::string& salt)         { recovery_salt_ = salt; }
    void set_recovery_master_key(const std::string& key)    { recovery_master_key_ = key; }

    void set_question_salt(const std::string& salt)         { question_salt_ = salt; }
    void set_question_master_key(const std::string& key)    { question_master_key_ = key; }
private:
    std::string salt_;
    std::string master_key_;

    std::string recovery_salt_;
    std::string recovery_master_key_;

    std::string question_salt_;
    std::string question_master_key_;

    std::string question_one_;
    std::string question_two_;
    std::string question_three_;
};
#endif

