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
                   const std::string& postid) {
    filename_ = filename;
    filepath_ = filepath;
    post_id_ = postid;
    chunk_count_ = 0;
    file_size_ = 0;
    deleted_ = false;
    shared_ = false;
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

ChunkInfo* FileInfo::GetChunkInfo(const std::string& chunkname) {
    if(chunks_.find(chunkname) != chunks_.end()) 
        return &chunks_[chunkname];
    return NULL;
}

void FileInfo::GetSerializedChunkData(std::string& out) const {
    ChunkMap::const_iterator itr = chunks_.begin();
    Json::Value chunk_list(Json::arrayValue);

    for(;itr != chunks_.end(); itr++) {
        Json::Value c;
        ChunkInfo ci = itr->second; // we copy because of the constness ... 
        jsn::SerializeObject(&ci, c);
        chunk_list.append(c);
    }

    Json::Value list;
    Json::StyledWriter writer;
    out = writer.write(chunk_list);
}

bool FileInfo::LoadSerializedChunkData(const std::string& data) {
    // only deserialize into empty vector
    if(chunks_.size() != 0)
        return false;
    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(data, root)) {
        return false;
    }

    Json::ValueIterator itr = root.begin();
    for(; itr != root.end(); itr++) {
        ChunkInfo ci;
        jsn::DeserializeObject(&ci, *itr);
        std::string name = ci.chunk_name();
        if(chunks_.find(name) == chunks_.end()) {
            chunks_[name] = ci;
        }
    }

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

void FileInfo::set_file_credentials(const Credentials& cred) { 
    file_credentials_ = cred; 
    set_file_credentials_key(cred.key());
    set_file_credentials_iv(cred.iv());
}

}//namespace
