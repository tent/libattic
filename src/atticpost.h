
#ifndef ATTICPOST_H_
#define ATTICPOST_H_
#pragma once

#include <string>

#include "post.h"

class AtticPost : public Post
{
public:
    AtticPost();
    ~AtticPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    void SetFilename(const std::string &name) { m_Name = name; }            
    void SetFilepath(const std::string &path) { m_Path = path; }            
    void SetMIME(const std::string &type) { m_Type = type; }
    void SetSize(const int size) { m_Size = size; }                         

private:
    // Attic specific post                       
    std::string m_Name; // Name of file
    std::string m_Path; // Relative file path within attic folder 
    std::string m_Type; // MIME Type (optional, if applicable)
    int m_Size; // Size of file, TODO:: checksum 
};


#endif

