
#include "chunkpost.h"

#include "constants.h"
#include "errorcodes.h"
#include "chunkinfo.h"

namespace attic { 

ChunkPost::ChunkPost() {
    set_type(cnst::g_attic_chunk_type);
}

ChunkPost::~ChunkPost() {}

// TODO :: rethink how chunk info objects are handled as a whole,
//         there are going to be alot of copies with this strategy
int ChunkPost::set_chunk_info_list(FileInfo::ChunkMap& list) {
    int status = ret::A_OK;

    std::map<std::string, ChunkInfo>::iterator itr = list.begin();
    for(;itr != list.end(); itr++) {
        // copy
        chunk_info_list_[itr->second.position()] = itr->second;
    }

    return status;
}

void ChunkPost::Serialize(Json::Value& root) {
    std::cout<<" Serializing chunk post " << std::endl;
    if(chunk_info_list_.size() > 0) {
        std::vector<std::string> serializedList;
        ChunkInfoList::iterator itr = chunk_info_list_.begin();

        Json::Value chunkval(Json::arrayValue);
        for(;itr != chunk_info_list_.end(); itr++) {
            Json::Value val(Json::objectValue);
            jsn::SerializeObject(&(itr->second), val);
            std::string chunkname = itr->second.chunk_name();
            chunkval.append(val);
        }
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
    Json::ValueIterator itr = chunkval.begin();
    for(; itr != chunkval.end(); itr++) {
        ChunkInfo ci;
        jsn::DeserializeObject(&ci, (*itr));
        chunk_info_list_[ci.position()] = ci;
    }
}

} //namespace
