
#ifndef FOLDERPOST_H_
#define FOLDERPOST_H_
#pragma once

#include <vector>
#include "post.h"
#include "folder.h"

class FolderPost : public Post
{
public:
    FolderPost();
    FolderPost(const Folder& folder);
    ~FolderPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    int PushBackFolderPost(FolderPost& post);

    void GetFileType(std::string& out) { out = m_FileType; }
    void GetRelativePath(std::string& out) { out = m_RelativePath; }
    
    void SetFileType(const std::string& type) { m_FileType = type; }
    void SetRelativePath(const std::string& path) { m_RelativePath = path; }

private:
    Folder  m_Folder;
    std::string m_RelativePath; // Relative path to file or folder
    std::string m_FileType;     // File or Folder
};

#endif

