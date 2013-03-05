#ifndef FOLDER_H_
#define FOLDER_H_
#pragma once

#include <string>
#include <map>

#include "jsonserializable.h"

class FolderEntry : public JsonSerializable
{
public:
    FolderEntry();
    FolderEntry( const std::string& postid,
                 const std::string& type,
                 const std::string& path); 

    ~FolderEntry();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void GetPostID(std::string& out) const { out = m_Postid; }
    void GetType(std::string& out) const { out = m_Type; }
    void GetPath(std::string& out) const { out = m_Path; }
    std::string GetPath(void) const { return m_Path; }

    void SetPostID(const std::string& id) { m_Postid = id; }
    void SetType(const std::string& type) { m_Type = type; }
    void SetPath(const std::string& path) { m_Path = path; }

private:
    std::string m_Postid; 
    std::string m_Type;   // file or folder
    std::string m_Path;
};

class Folder : public FolderEntry
{
public:
    Folder();
    ~Folder();

    virtual void Serialize(Json::Value& root);
    void SerializeContents(std::string& out);
    virtual void Deserialize(Json::Value& root);
    void DeserializeContents(const std::string& in);

    void PushBackEntry(const FolderEntry& entry) { m_Entries[entry.GetPath()] = entry; }
    bool GetFolderEntry(const std::string& path, FolderEntry& entry);


private:
    typedef std::map<std::string, FolderEntry> EntryList;
    EntryList m_Entries;
};

#endif

