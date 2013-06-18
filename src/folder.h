#ifndef FOLDER_H_
#define FOLDER_H_
#pragma once

#include <string>
#include <vector>

#include "jsonserializable.h"

namespace attic { 

class Folder {
public:
    Folder() {
        deleted_ = false;
    }
    ~Folder() {}

    const std::string& foldername() const           { return foldername_; }
    const std::string& folder_post_id() const       { return folder_post_id_; }
    const std::string& parent_post_id() const       { return parent_post_id_; } 
    bool deleted() const                            { return deleted_; }

    void set_foldername(const std::string& path)    { foldername_ = path; } 
    void set_folder_post_id(const std::string& id)  { folder_post_id_ = id; } 
    void set_parent_post_id(const std::string& id)  { parent_post_id_ = id; }
    void set_deleted(bool del)                      { deleted_ = del; }
private:
    std::string foldername_;
    std::string folder_post_id_;
    std::string parent_post_id_;
    bool deleted_;
};

}//namespace
#endif

