
#include "chunkpost.h"

#include "constants.h"
#include "errorcodes.h"
#include "chunkinfo.h"

namespace attic { 

ChunkPost::ChunkPost() {
    set_type(cnst::g_attic_chunk_type);
}

ChunkPost::~ChunkPost() { }

// TODO :: rethink how chunk info objects are handled as a whole,
//         there are going to be alot of copies with this strategy
int ChunkPost::SetChunkInfoList(std::map<std::string, ChunkInfo>& List) {
    int status = ret::A_OK;

    std::map<std::string, ChunkInfo>::iterator itr = List.begin();
    for(;itr != List.end(); itr++) {
        // copy
        m_ChunkList.push_back(itr->second);
    }

    return status;
}

void ChunkPost::Serialize(Json::Value& root) {
    if(m_ChunkList.size() > 0) {
        std::vector<std::string> serializedList;
        std::vector<ChunkInfo>::iterator itr = m_ChunkList.begin();

        std::string val;
        for(;itr != m_ChunkList.end(); itr++) {
            val.clear();
            jsn::SerializeObject(&(*itr), val);
            serializedList.push_back(val);
        }

        //std::string cval;
        Json::Value chunkval;
        jsn::SerializeVector(serializedList, chunkval);
        //jsn::SerializeJsonValue(chunkval, cval);

        //root["chunks"] = chunkval;
        set_content("chunks", chunkval);
    }

    Post::Serialize(root);
}

void ChunkPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    //std::string cval;
    Json::Value chunkval;
    std::vector<std::string> serializedList;

    get_content("chunks", chunkval);
    jsn::DeserializeIntoVector(chunkval, serializedList);

    if(serializedList.size() > 0) {
        std::vector<std::string>::iterator itr = serializedList.begin();

        for(;itr != serializedList.end(); itr++) {
            ChunkInfo ci;
            jsn::DeserializeObject(&ci, (*itr));
            m_ChunkList.push_back(ci); // copy
        }
    }
}

} //namespace
