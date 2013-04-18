#ifndef FOLDER_H_
#define FOLDER_H_
#pragma once

#include <string>

namespace attic { 

class Folder {
public:
    Folder() {}
    ~Folder() {}

    const std::string& folderpath() const       { return folderpath_; }
    const std::string& manifest_id() const      { return manifest_id_; }
    const std::string& folder_post_id() const   { return folder_post_id_; }

    void set_folderpath(const std::string& path)    { folderpath_ = path; } 
    void set_manifest_id(const std::string& id)     { manifest_id_ = id; }
    void set_folder_post_id(const std::string& id)  { folder_post_id_ = id; } 
private:
    std::string folderpath_;
    std::string manifest_id_;
    std::string folder_post_id_;
};

}//namespace
#endif

