
#include "atticpost.h"

#include <stdio.h>
#include <cbase64.h>

#include "constants.h"

AtticPost::AtticPost()
{
    // PostType
    //SetPostType(cnst::g_szAtticPostType); // old type
    SetPostType(cnst::g_szFileMetadataPostType);
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
    
    Json::Value chunkposts;
    JsonSerializer::SerializeVector(chunkposts, m_ChunkPosts);
    std::string chunkval;
    JsonSerializer::SerializeJsonValue(chunkposts, chunkval);
    SetContent("chunk_posts", chunkval);

    Json::Value chunkids;
    JsonSerializer::SerializeVector(chunkids, m_ChunkIds);
    std::string idval;
    JsonSerializer::SerializeJsonValue(chunkids, idval);
    SetContent("chunk_ids", idval);

    SetContent( "kdata", 
                cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_KeyData.c_str()), 
                                     m_KeyData.size())
              );

    SetContent( "vdata",
                cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_IvData.c_str()),
                                     m_IvData.size())
              );

    Post::Serialize(root);
}

void AtticPost::Deserialize(Json::Value& root)
{
    Post::Deserialize(root);

    // Extract attic post specific things from post content
    GetContent("name", m_Name);
    GetContent("path", m_Path);
    GetContent("type", m_MimeType);

    std::string size;
    GetContent("size", size);
    m_Size = atoi(size.c_str());

    std::string chunkval, idval;
    GetContent("chunk_posts", chunkval);
    GetContent("chunk_ids", idval);

    Json::Value cp, ci;
    JsonSerializer::DeserializeJsonValue(cp, chunkval);
    JsonSerializer::DeserializeJsonValue(ci, idval);

    JsonSerializer::DeserializeIntoVector(cp, m_ChunkPosts);
    JsonSerializer::DeserializeIntoVector(ci, m_ChunkIds);

    GetContent("kdata", m_KeyData);
    m_KeyData = cb64::base64_decode(m_KeyData);
    GetContent("vdata", m_IvData);
    m_IvData = cb64::base64_decode(m_IvData);
}


