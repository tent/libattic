
#include "chunkpost.h"

#include "constants.h"
#include "errorcodes.h"
#include "chunkinfo.h"

ChunkPost::ChunkPost()
{
    SetPostType(cnst::g_szChunkStorePostType);
}

ChunkPost::~ChunkPost()
{

}

// TODO :: rethink how chunk info objects are handled as a whole,
//         there are going to be alot of copies with this strategy
int ChunkPost::SetChunkInfoList(std::map<std::string, ChunkInfo>& List)
{
    int status = ret::A_OK;

    std::map<std::string, ChunkInfo>::iterator itr = List.begin();
    for(;itr != List.end(); itr++)
    {
        // copy
        m_ChunkList.push_back(itr->second);
    }

    return status;
}

void ChunkPost::Serialize(Json::Value& root)
{
    if(m_ChunkList.size() > 0)
    {
        std::vector<std::string> serializedList;
        std::vector<ChunkInfo>::iterator itr = m_ChunkList.begin();

        std::string val;
        for(;itr != m_ChunkList.end(); itr++)
        {
            val.clear();
            jsn::SerializeObject(&(*itr), val);
            serializedList.push_back(val);
        }

        std::string cval;
        Json::Value chunkval;
        jsn::SerializeVector(chunkval, serializedList);
        jsn::SerializeJsonValue(chunkval, cval);

        //root["chunks"] = chunkval;
        SetContent("chunks", cval);
    }

    Post::Serialize(root);
}

void ChunkPost::Deserialize(Json::Value& root)
{
    Post::Deserialize(root);

    std::string cval;
    Json::Value chunkval;
    std::vector<std::string> serializedList;

    GetContent("chunks", cval);

    jsn::DeserializeJsonValue(chunkval, cval);
    jsn::DeserializeIntoVector(chunkval, serializedList);

    if(serializedList.size() > 0)
    {
        std::vector<std::string>::iterator itr = serializedList.begin();

        for(;itr != serializedList.end(); itr++)
        {
            ChunkInfo ci;
            jsn::DeserializeObject(&ci, (*itr));
            m_ChunkList.push_back(ci); // copy
        }
    }

}


