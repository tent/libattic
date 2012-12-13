
#include "metastorepost.h"

#include "constants.h"

MetaStorePost::MetaStorePost()
{
    SetPostType(g_szAtticMetaStorePostType);
}

MetaStorePost::~MetaStorePost()
{

}

void MetaStorePost::Serialize(Json::Value& root)
{
    SetContent("attic_root", m_AtticRoot);

    Post::Serialize(root);
}

void MetaStorePost::Deserialize(Json::Value& root)
{
    Post::Deserialize(root);

    GetContent("attic_root", m_AtticRoot);
}


