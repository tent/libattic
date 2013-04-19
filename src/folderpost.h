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
    void set_folder(const Folder& folder)           { folder_ = folder; }

private:
    Folder  folder_;
};

} //namespace
#endif

