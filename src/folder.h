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
    const std::string& local_alias() const          { return local_alias_; }
    bool deleted() const                            { return deleted_; }

    void set_foldername(const std::string& path)    { foldername_ = path; } 
    void set_folder_post_id(const std::string& id)  { folder_post_id_ = id; } 
    void set_parent_post_id(const std::string& id)  { parent_post_id_ = id; }
    void set_local_alias(const std::string& alias)  { local_alias_ = alias; } 
    void set_deleted(bool del)                      { deleted_ = del; }

    bool has_alias() { return !local_alias_.empty(); }
    void clear_alias() { local_alias_.clear(); }
private:
    std::string local_alias_;   // Local alias of the file, does not exist in the folder post

    std::string foldername_;
    std::string folder_post_id_;
    std::string parent_post_id_;
    bool deleted_;
};

}//namespace
#endif

