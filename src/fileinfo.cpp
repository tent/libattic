
#include "fileinfo.h"

#include <fstream>
#include <vector>
#include <stdio.h>

#include "utils.h"

namespace attic { 

FileInfo::FileInfo() {
    deleted_ = 0;
}

FileInfo::FileInfo(const std::string& filename,
                   const std::string& filepath,
                   const std::string& postid,
                   const std::string& chunkpostid) {
    FileInfo::FileInfo();

    filename_ = filename;
    filepath_ = filepath;
    post_id_ = postid;
    chunk_post_id_ = chunkpostid;
}

FileInfo::~FileInfo() {}

bool FileInfo::InitializeFile(const std::string &filepath) {
    // Check if Valid File
    //
    // Set filepath
    filepath_ = filepath;
    // Extract Filename
    ExtractFilename(filepath, filename_); 
    // Check file size
    file_size_ = utils::CheckFilesize(filepath);

    if(!file_size_)
        return false;

    return true;
}

void FileInfo::ExtractFilename(const std::string &filepath, std::string &out) {
    std::string name;
    unsigned int size = filepath.size();
    if(size) {
        // Check if passed a directory
        if(filepath[size-1] != '/') {
            std::vector<std::string> out;
            utils::SplitString(filepath, '/', out);
            if(out.size()) {
                name = out[out.size()-1];
            }
        }
    }
    out = name;
}

int FileInfo::PushChunkBack(ChunkInfo& chunk) {
    // Will overwrite what was already in there.
    int status = ret::A_OK;

    // Check if entry exists
    std::string chunkname = chunk.chunk_name();

    chunks_[chunkname] = chunk;
    chunk_count_ = chunks_.size();

    return status;
}

void FileInfo::PushBackAlias(const std::string& alias) {
    past_aliases_[alias] = false;
}

bool FileInfo::HasAlias(const std::string& alias) {
    if(past_aliases_.find(alias) != past_aliases_.end())
        return true;
    return false;
}

void FileInfo::GetSerializedAliasData(std::string& out) const {
    std::map<std::string, bool>::const_iterator itr = past_aliases_.begin();

    Json::Value alias_array(Json::arrayValue);
    for(;itr != past_aliases_.end(); itr++) {
        alias_array.append(itr->first);
    }

    Json::StyledWriter writer;
    out = writer.write(alias_array);
}

ChunkInfo* FileInfo::GetChunkInfo(const std::string& chunkname) {
    std::cout<<" FI CHUNK COUNT : " << chunks_.size() << std::endl;
    if(chunks_.find(chunkname) != chunks_.end()) 
        return &chunks_[chunkname];
    return NULL;
}

void FileInfo::GetSerializedChunkData(std::string& out) const {
    ChunkMap::const_iterator itr = chunks_.begin();
    std::vector<std::string> chunkList;

    std::string chunk;
    for(;itr != chunks_.end(); itr++) {
        ChunkInfo ci = itr->second;
        chunk.clear();
        jsn::SerializeObject(&ci, chunk);
        chunkList.push_back(chunk);
    }

    if(chunkList.size() > 0) {
        Json::Value val;
        jsn::SerializeVector(chunkList, val);

        Json::StyledWriter writer;
        out = writer.write(val);
    }
}

bool FileInfo::LoadSerializedAliasData(const std::string& data) {
    Json::Value root;
    Json::Reader reader;
    if(reader.parse(data, root)) {
        Json::ValueIterator itr = root.begin();
        for(; itr != root.end(); itr++) {
            past_aliases_[itr.key().asString()] = false;
        }
    }
    return false;
}

bool FileInfo::LoadSerializedChunkData(const std::string& data) {
    // only deserialize into empty vector
    std::cout<<" LOADING SERIALIZED CHUNK DATA " << std::endl;
    if(chunks_.size() != 0)
        return false;

    std::cout<<" RAW DATA : " << data << std::endl;

    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(data, root)) {
        std::cout<<" FAILED TO PARSE " << std::endl;
        return false;
    }

    std::vector<std::string> chunkList;
    jsn::DeserializeIntoVector(root, chunkList);

    std::cout<<" CHUNK LIST SIZE : " << chunkList.size() << std::endl;
    std::vector<std::string>::iterator itr = chunkList.begin();

    for(;itr != chunkList.end(); itr++) {
        ChunkInfo* ci = new ChunkInfo();
        jsn::DeserializeObject(ci, *itr);

        std::string name = ci->chunk_name();
        if(chunks_.find(name) == chunks_.end()) {
            chunks_[name] = *ci;
        }

        delete ci;
    }

    return true;
}

bool FileInfo::LoadSerializedChunkPost(const std::string& data) {
    std::cout<<" Loading Serialize Chunk Post POST " << std::endl;

    return true;
}

bool FileInfo::HasEncryptedKey() {
    if(encrypted_key_.empty())
        return false;
    return true;
}

bool FileInfo::DoesChunkExist(const std::string& chunk_name) {
    if(chunks_.find(chunk_name) != chunks_.end())
        return true;
    return false;
}

}//namespace
