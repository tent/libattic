
#ifndef FOLDERPOST_H_
#define FOLDERPOST_H_
#pragma once

#include "post.h"

class FolderPost : public Post
{
public:
    FolderPost();
    ~FolderPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

private:
};

#endif

