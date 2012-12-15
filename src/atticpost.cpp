
#include "atticpost.h"

#include <stdio.h>

#include "constants.h"

AtticPost::AtticPost()
{
    // PostType
    SetPostType(cnst::g_szAtticPostType);
}

AtticPost::~AtticPost()
{

}

void AtticPost::Serialize(Json::Value& root)
{
    // Insert post specific things into post content
    SetContent("name", m_Name); 
    SetContent("path", m_Path);
    //SetContent("type", m_Type)
    char buf[256];
    snprintf(buf, 256, "%d", m_Size);
    SetContent("size", buf);

    Post::Serialize(root);
}

void AtticPost::Deserialize(Json::Value& root)
{
    Post::Deserialize(root);

    // Extract attic post specific things from post content
    GetContent("name", m_Name);
    GetContent("path", m_Path);
    GetContent("type", m_Type);

    std::string size;
    GetContent("size", size);
    m_Size = atoi(size.c_str());
}


