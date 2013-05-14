#ifndef FOLDERPOST_H_
#define FOLDERPOST_H_
#pragma once

#include <string>
#include <vector>
#include "folder.h"
#include "post.h"

namespace attic { 

class FolderPost : public Post {
    void SerializePastAliases(Json::Value& val);
    void DeserializePastAliases(Json::Value& val);
public:
    FolderPost();
    FolderPost(const Folder& folder);
    ~FolderPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);


    void PushBackAlias(const std::string& alias)        { past_aliases_.push_back(alias); }
    std::vector<std::string> GetPastAliases() const     { return past_aliases_; }

    const Folder& folder() const                { return folder_; }
    void set_folder(const Folder& folder)       { folder_ = folder; }

private:
    std::vector<std::string> past_aliases_;

    Folder  folder_;
};

} //namespace
#endif

