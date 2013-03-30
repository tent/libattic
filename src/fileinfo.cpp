
#include "fileinfo.h"

#include <fstream>
#include <vector>
#include <stdio.h>

#include "utils.h"

namespace attic { 

FileInfo::FileInfo() {
    m_PostVersion = 0;
    m_Deleted = 0;
}

FileInfo::FileInfo(const std::string& filename,
                   const std::string& filepath,
                   const std::string& postid,
                   const std::string& chunkpostid) {
    FileInfo::FileInfo();

    m_Filename = filename;
    m_Filepath = filepath;
    m_PostID = postid;
    m_ChunkPostID = chunkpostid;
}

FileInfo::~FileInfo()
{

}

bool FileInfo::InitializeFile(const std::string &filepath)
{
    // Check if Valid File
    //
    // Set filepath
    m_Filepath = filepath;
    // Extract Filename
    ExtractFilename(filepath, m_Filename); 
    // Check file size
    m_FileSize = utils::CheckFilesize(filepath);

    if(!m_FileSize)
        return false;

    return true;
}

void FileInfo::ExtractFilename(const std::string &filepath, std::string &out)
{
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

int FileInfo::PushChunkBack(ChunkInfo& Chunk)
{
    // Will overwrite what was already in there.
    int status = ret::A_OK;

    // Check if entry exists
    std::string chunkname;
    Chunk.GetChunkName(chunkname);

    m_Chunks[chunkname] = Chunk;
    m_ChunkCount = m_Chunks.size();


    return status;
}

ChunkInfo* FileInfo::GetChunkInfo(const std::string& chunkname)
{
    if(m_Chunks.find(chunkname) != m_Chunks.end()) 
        return &m_Chunks[chunkname];
    return NULL;
}

void FileInfo::GetSerializedChunkData(std::string& out) const
{
    ChunkMap::const_iterator itr = m_Chunks.begin();
    std::vector<std::string> chunkList;

    std::string chunk;
    for(;itr != m_Chunks.end(); itr++) {
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

bool FileInfo::LoadSerializedChunkData(const std::string& data)
{
    // only deserialize into empty vector
    if(m_Chunks.size() != 0)
        return false;

    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(data, root))
        return false;

    std::vector<std::string> chunkList;
    jsn::DeserializeIntoVector(root, chunkList);

    std::vector<std::string>::iterator itr = chunkList.begin();

    for(;itr != chunkList.end(); itr++) {
        ChunkInfo* ci = new ChunkInfo();
        jsn::DeserializeObject(ci, *itr);

        std::string name;
        ci->GetChunkName(name);
        if(m_Chunks.find(name) == m_Chunks.end()) {
            m_Chunks[name] = *ci;
        }

        delete ci;
    }

    return true;
}

bool FileInfo::LoadSerializedChunkPost(const std::string& data)
{

    return true;
}

bool FileInfo::HasEncryptedKey()
{
    if(m_EncryptedKey.empty())
        return false;
    return true;
}

}//namespace
