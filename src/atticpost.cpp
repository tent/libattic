
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
    std::cout<<" \t\t serialize " << std::endl;
    // Insert post specific things into post content
    //SetContent("name", m_Name); 
    //SetContent("path", m_Path);
    //SetContent("type", m_Type)
    char buf[256] = {'\0'};
    snprintf(buf, 256, "%d", m_Size);
    //SetContent("size", buf);
    //SetContent("chunk_name", m_ChunkName);

    Json::Value content;
    content["name"] = m_Name;
    content["path"] = m_Path;
    content["size"] = buf;

    std::cout<<" \t\t serialize " << std::endl;
    //content["chunk_name"] = m_ChunkName; // depricated

    char del[256] = {'\0'};
    snprintf(del, 256, "%d", m_Deleted);
    //SetContent("deleted", del);
    content["deleted"] = del;
    
    std::cout<<" \t\t serialize " << std::endl;
    Json::Value chunkposts;//(Json::objectValue);
    jsn::SerializeVector(chunkposts, m_ChunkPosts);
    //std::string chunkval;
    //jsn::SerializeJsonValue(chunkposts, chunkval);
    //SetContent("chunk_posts", chunkval);
   
    std::cout<<" \t\t serialize " << std::endl;
    content["chunk_posts"] = chunkposts;

    std::cout<<" \t\t serialize " << std::endl;
    Json::Value chunkids;
    jsn::SerializeVector(chunkids, m_ChunkIds);
    //std::string idval;
    //jsn::SerializeJsonValue(chunkids, idval);
    //SetContent("chunk_ids", idval);
    content["chunk_ids"] = chunkids;
    std::string key_data = cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_KeyData.c_str()), 
                                     m_KeyData.size());
    std::cout<<" \t\t serialize " << std::endl;
    std::cout<<" KEY DATA : " << key_data << std::endl;
    content["kdata"] = key_data;

    std::cout<<" \t\t serialize " << std::endl;
    std::string iv_data = cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_IvData.c_str()),
                                     m_IvData.size());
    std::cout<<" \t\t serialize " << std::endl;
    std::cout<< "IV DATA : " << iv_data << std::endl;
    content["vdata"] = iv_data;

    std::cout<<" \t\t serialize " << std::endl;
    /*
    SetContent( "kdata", 
                cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_KeyData.c_str()), 
                                     m_KeyData.size())
              );

    SetContent( "vdata",
                cb64::base64_encode( reinterpret_cast<const unsigned char*>(m_IvData.c_str()),
                                     m_IvData.size())
              );
              */
    SetContent("chunk_content", content);

    std::cout<<" \t\t serialize " << std::endl;
    Post::Serialize(root);
}

void AtticPost::Deserialize(Json::Value& root)
{
    std::cout<<" deserializing here " << std::endl;
    Post::Deserialize(root);

    std::cout<<" deserializing here " << std::endl;

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

    //Json::Value chunkids = content["chunk_ids"];
    //jsn::DeserializeJsonValue(chunkids, content["chunk_ids"]);
    jsn::DeserializeIntoVector(content["chunk_ids"], m_ChunkIds);

    m_KeyData = content["kdata"].asString();
    m_IvData = content["vdata"].asString();

    m_KeyData = cb64::base64_decode(m_KeyData);
    m_IvData = cb64::base64_decode(m_IvData);

    // Extract attic post specific things from post content
    /*
    GetContent("name", m_Name);
    GetContent("path", m_Path);
    GetContent("type", m_MimeType);

    std::string size;
    GetContent("size", size);
    m_Size = atoi(size.c_str());
    
    std::string del;
    GetContent("deleted", del);
    m_Deleted = atoi(del.c_str());


    std::string chunkval, idval;
    //GetContent("chunk_name", m_ChunkName); // depricated
    //GetContent("chunk_posts", chunkval);
    GetContent("chunk_ids", idval);
    std::cout<<" deserializing here " << std::endl;

    Json::Value cp, ci;
    jsn::DeserializeJsonValue(cp, chunkval);
    jsn::DeserializeJsonValue(ci, idval);
    std::cout<<" deserializing here " << std::endl;

    jsn::DeserializeIntoVector(cp, m_ChunkPosts);
    jsn::DeserializeIntoVector(ci, m_ChunkIds);
    std::cout<<" deserializing here " << std::endl;


    GetContent("kdata", m_KeyData);
    m_KeyData = cb64::base64_decode(m_KeyData);
    GetContent("vdata", m_IvData);
    m_IvData = cb64::base64_decode(m_IvData);

    */
}

