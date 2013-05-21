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

    const std::string& folderpath() const           { return folderpath_; }
    const std::string& folder_post_id() const       { return folder_post_id_; }
    const std::string& parent_post_id() const       { return parent_post_id_; } 

    void set_folderpath(const std::string& path)    { folderpath_ = path; } 
    void set_folder_post_id(const std::string& id)  { folder_post_id_ = id; } 
    void set_parent_post_id(const std::string& id)  { parent_post_id_ = id; }

private:
    std::string folderpath_;
    std::string folder_post_id_;
    std::string parent_post_id_;
};

}//namespace
#endif

