
#include "fileinfo.h"

#include <fstream>
#include <vector>
#include <stdio.h>

#include "utils.h"

FileInfo::FileInfo()
{

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
    if(size)
    {
        // Check if passed a directory
        if(filepath[size-1] != '/')
        {
            std::vector<std::string> out;
            utils::SplitString(filepath, '/', out);
            if(out.size())
            {
                name = out[out.size()-1];
            }
        }
    }
    out = name;
}

int FileInfo::PushChunkBack(ChunkInfo& Chunk)
{
    int status = ret::A_OK;

    // Check if entry exists
    std::string chunkname;
    Chunk.GetChunkName(chunkname);
    if(m_Chunks.find(chunkname) == m_Chunks.end())
    {
        m_Chunks[chunkname] = Chunk;
        m_ChunkCount = m_Chunks.size();
    }
    else
    {
        status = ret::A_FAIL_DUPLICATE_ENTRY;
    }

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
    for(;itr != m_Chunks.end(); itr++)
    {
        ChunkInfo ci = itr->second;
        chunk.clear();
        JsonSerializer::SerializeObject(&ci, chunk);
        chunkList.push_back(chunk);
    }

    if(chunkList.size() > 0)
    {
        Json::Value val;
        JsonSerializer::SerializeVector(val, chunkList);

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
    JsonSerializer::DeserializeIntoVector(root, chunkList);

    std::vector<std::string>::iterator itr = chunkList.begin();

    for(;itr != chunkList.end(); itr++)
    {
        ChunkInfo* ci = new ChunkInfo();
        JsonSerializer::DeserializeObject(ci, *itr);

        std::string name;
        ci->GetChunkName(name);
        if(m_Chunks.find(name) == m_Chunks.end())
        {
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

