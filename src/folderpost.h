
#ifndef FOLDERPOST_H_
#define FOLDERPOST_H_
#pragma once

#include <vector>
#include "post.h"

class FolderPost : public Post
{
public:
    FolderPost();
    ~FolderPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    int PushBackFolderPost(FolderPost& post);

    void GetFileType(std::string& out) { out = m_FileType; }
    
    void SetFileType(const std::string& type) { m_FileType = type; }
private:
    std::vector<FolderPost> m_Children;
    std::string m_FileType;
};

#endif

