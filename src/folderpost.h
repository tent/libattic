#ifndef FOLDERPOST_H_
#define FOLDERPOST_H_
#pragma once

#include <string>
#include <vector>
#include "folder.h"
#include "post.h"

namespace attic { 

class FolderPost : public Post {
public:
    FolderPost();
    FolderPost(const Folder& folder);
    ~FolderPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    const Folder& folder() const                { return folder_; }
    const std::string& relative_path() const    { return relative_path_; }
    const std::string& file_type() const        { return file_type_; }

    void set_folder(const Folder& folder)           { folder_ = folder; }
    void set_relative_path(const std::string& path) { relative_path_ = path; }
    void set_file_type(const std::string& type)     { file_type_ = type; }

private:
    Folder  folder_;
    std::string relative_path_; // Relative path to file or folder
    std::string file_type_;     // File or Folder
};

} //namespace
#endif

