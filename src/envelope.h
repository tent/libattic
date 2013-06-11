#ifndef ENVELOPE_H_
#define ENVELOPE_H_
#pragma once

#include <map>
#include <string>
#include <deque>
#include "post.h"

namespace attic { 

class Pages : public JsonSerializable {
public:
    Pages() {}
    ~Pages() {}

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    const std::string& first() const { return first_; }
    const std::string& prev() const { return prev_; }
    const std::string& next() const { return next_; }
    const std::string& last() const { return last_; }
    std::string asString() const;
private:
    std::string first_;
    std::string prev_;
    std::string next_;
    std::string last_;
};

class Envelope : public JsonSerializable { 
public:
    typedef std::deque<Post> PostQueue;

    Envelope() {}
    ~Envelope() {}

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    const Pages& pages() const { return pages_; }

    const std::string& data() const { return data_; }
    const Post& post() const { return post_; }

    PostQueue* posts() { return &posts_; }
private:
    Pages                               pages_;
    PostQueue                           posts_;
    std::deque<Mention>                 mentions_;
    std::deque<Version>                 versions_;
    std::deque<Reference>               reference_;
    std::map<std::string, Profile>      profiles_;

    Post                         post_;      // Singleton post

    std::string data_;
};

}//namespace
#endif

