#ifndef FOLDER_H_
#define FOLDER_H_
#pragma once

#include <string>
#include <vector>

#include "jsonserializable.h"

namespace attic { 

class Folder {
public:
    Folder() {}
    ~Folder() {}

    void PushBackAlias(const std::string& alias)    { alias_data_.push_back(alias); }
    std::vector<std::string>* alias_data()          { return &alias_data_; }

    const std::string& folderpath() const           { return folderpath_; }
    const std::string& folder_post_id() const       { return folder_post_id_; }
    const std::string& parent_post_id() const       { return parent_post_id_; } 

    void set_folderpath(const std::string& path)    { folderpath_ = path; } 
    void set_folder_post_id(const std::string& id)  { folder_post_id_ = id; } 
    void set_parent_post_id(const std::string& id)  { parent_post_id_ = id; }
    void set_alias_data(const std::vector<std::string>& v) { alias_data_ = v; }

    std::string SerializeAliasData() {
        Json::Value s;
        jsn::SerializeVector(alias_data_, s);
        std::string out;
        jsn::SerializeJsonValue(s, out);
        return out;
    }

    void DeserializeAliasData(const std::string& data) {
        Json::Value s;
        jsn::DeserializeJson(data, s);
        jsn::DeserializeIntoVector(s, alias_data_);
    }

private:
    std::vector<std::string> alias_data_;
    std::string folderpath_;
    std::string folder_post_id_;
    std::string parent_post_id_;
};

}//namespace
#endif

