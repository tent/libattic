
#ifndef ATTICPOST_H_
#define ATTICPOST_H_
#pragma once

#include <string>
#include <vector>

#include "post.h"

class FileInfo;

class AtticPost : public Post
{
public:
    AtticPost();
    ~AtticPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    void AtticPostSetFilename(const std::string &name)      { m_Name = name; }            
    void AtticPostSetFilepath(const std::string &path)      { m_Path = path; }            
    void AtticPostSetChunkName(const std::string &name)     { m_ChunkName = name; } 
    void AtticPostSetMIME(const std::string &type)          { m_MimeType = type; }
    void AtticPostSetSize(const int size)                   { m_Size = size; }
    void AtticPostSetKeyData(const std::string& data)       { m_KeyData = data; }
    void AtticPostSetIvData(const std::string& data)        { m_IvData = data; }

    void GetAtticPostFilename(std::string& out) const   { out = m_Name; }
    void GetAtticPostFilepath(std::string& out) const   { out = m_Path; }
    void GetAtticPostChunkName(std::string& out) const  { out = m_ChunkName; }
    int GetAtticPostSize() const                        { return m_Size; }
    void GetAtticPostKeyData(std::string& out) const    { out = m_KeyData; }
    void GetAtticPostIvData(std::string& out) const     { out = m_IvData; }

    void PushBackChunkPostId(const std::string& postId) { m_ChunkPosts.push_back(postId); }
    void PushBackChunkIdentifier(const std::string& id) { m_ChunkIds.push_back(id); }

    std::vector<std::string>* GetChunkPosts() { return &m_ChunkPosts; }

private:
    std::vector<std::string> m_ChunkPosts;
    std::vector<std::string> m_ChunkIds;

    // Attic specific post                       
    std::string m_Name; // Name of file
    std::string m_ChunkName; // Name of chunk
    std::string m_Path; // Relative file path within attic folder 
    std::string m_MimeType; // MIME Type (optional, if applicable)
    std::string m_KeyData;
    std::string m_IvData;
    int m_Size; // Size of file, TODO:: checksum 
};


#endif

