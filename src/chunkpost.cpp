#include "chunkpost.h"

#include "constants.h"
#include "errorcodes.h"
#include "chunkinfo.h"

namespace attic { 

ChunkPost::ChunkPost() {
    set_type(cnst::g_attic_chunk_type);
    group_ = -1;
}

ChunkPost::~ChunkPost() {}

// TODO :: rethink how chunk info objects are handled as a whole,
//         there are going to be alot of copies with this strategy
int ChunkPost::set_chunk_info_list(FileInfo::ChunkMap& list) {
    int status = ret::A_OK;

    chunk_map_ = list;
    std::map<std::string, ChunkInfo>::iterator itr = list.begin();
    for(;itr != list.end(); itr++) {
        chunk_info_list_[itr->second.position()] = itr->second;
    }

    return status;
}

void ChunkPost::PushBackChunkInfo(const ChunkInfo& ci, const unsigned int position) {
    chunk_info_list_[position] = ci;
    chunk_map_[ci.chunk_name()] = ci;
}

bool ChunkPost::HasChunk(const std::string& name) { 
    if(chunk_map_.find(name) != chunk_map_.end())
        return true;
    return false;
}

ChunkInfo* ChunkPost::GetChunkInfo(const std::string& name) {
    FileInfo::ChunkMap::iterator itr = chunk_map_.find(name);
    if(itr != chunk_map_.end())
        return &itr->second;
    return NULL;
}

bool ChunkPost::GetChunkInfo(const std::string& name, ChunkInfo& out) {
    if(HasChunk(name)) {
        FileInfo::ChunkMap::iterator itr = chunk_map_.find(name);
        if(itr != chunk_map_.end()) {
            out =itr->second;
            return true;
        }
    }
    return false;
}

void ChunkPost::Serialize(Json::Value& root) {
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

        Json::Value info(Json::objectValue);
        info["group_number"] = group_;
        set_content("chunks_info", info);
    }

    Post::Serialize(root);
}

void ChunkPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    //std::string cval;
    Json::Value chunkval(Json::arrayValue);
    std::vector<std::string> serializedList;

    get_content("chunks", chunkval);
    Json::ValueIterator itr = chunkval.begin();
    for(; itr != chunkval.end(); itr++) {
        ChunkInfo ci;
        jsn::DeserializeObject(&ci, (*itr));
        chunk_info_list_[ci.position()] = ci;
        chunk_map_[ci.chunk_name()] = ci;
    }

    Json::Value info(Json::objectValue);
    get_content("chunks_info", info);
    group_ =  info["group_number"].asInt(); 
}

} //namespace
