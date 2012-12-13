
#ifndef METASTOREPOST_H_
#define METASTOREPOST_H_
#pragma once

#include <string>
#include "post.h"

class MetaStorePost : public Post
{

public:
    MetaStorePost();
    ~MetaStorePost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    void SetAtticRoot(const std::string &root) { m_AtticRoot = root; }

private:
    std::string m_AtticRoot;

};



#endif

