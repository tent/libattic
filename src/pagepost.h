#ifndef PAGEPOST_H_
#define PAGEPOST_H_
#pragma once

#include <string>
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
private:
    std::string first_;
    std::string prev_;
    std::string next_;
    std::string last_;
};

class PagePost : public Post { 
public:
    PagePost() {}
    ~PagePost() {}

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    const Pages& pages() const { return pages_; }
    const std::string& data() const { return data_; }
private:
    Pages pages_;
    std::string data_;
};

}//namespace
#endif

