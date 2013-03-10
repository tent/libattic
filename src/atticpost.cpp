
#include "atticpost.h"

#include <stdio.h>
#include <cbase64.h>

#include "fileinfo.h"
#include "constants.h"
#include "errorcodes.h"

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
    char buf[256] = {'\0'};
    snprintf(buf, 256, "%d", m_Size);

    Json::Value content;
    content["name"] = m_Name;
    content["path"] = m_Path;
    content["size"] = buf;

    char del[256] = {'\0'};
    snprintf(del, 256, "%d", m_Deleted);
    content["deleted"] = del;
    
    Json::Value chunkposts;//(Json::objectValue);
    jsn::SerializeVector(chunkposts, m_ChunkPosts);
   
    content["chunk_posts"] = chunkposts;

    Json::Value chunkids;
    jsn::SerializeVector(chunkids, m_ChunkIds);
    content["chunk_ids"] = chunkids;
    std::string key_data = cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_KeyData.c_str()), 
                                     m_KeyData.size());
    content["kdata"] = key_data;

    std::string iv_data = cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_IvData.c_str()),
                                     m_IvData.size());
    content["vdata"] = iv_data;

    SetContent("chunk_content", content);

    Post::Serialize(root);
}

void AtticPost::Deserialize(Json::Value& root)
{
    Post::Deserialize(root);

    Json::Value content;
    GetContent("chunk_content", content);
    m_Name = content.get("name", "").asString();
    m_Path = content.get("path", "").asString();
    m_MimeType = content.get("type", "").asString();
    std::string size = content.get("size", "").asString();
    m_Size = atoi(size.c_str());
    std::string del = content.get("deleted", "").asString();
    m_Deleted = atoi(del.c_str());

    Json::Value chunkposts;
    jsn::DeserializeIntoVector(content["chunk_posts"], m_ChunkPosts);
    jsn::DeserializeIntoVector(content["chunk_ids"], m_ChunkIds);

    m_KeyData = content["kdata"].asString();
    m_IvData = content["vdata"].asString();

    m_KeyData = cb64::base64_decode(m_KeyData);
    m_IvData = cb64::base64_decode(m_IvData);

}

