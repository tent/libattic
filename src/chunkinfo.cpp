#include "chunkinfo.h"

ChunkInfo::ChunkInfo()
{

}

ChunkInfo::ChunkInfo(const std::string& chunkName, const std::string& checkSum)
{
    m_ChunkName = chunkName;
    m_CheckSum = checkSum;
}

ChunkInfo::~ChunkInfo()
{

}

void ChunkInfo::Serialize(Json::Value& root)
{
    root["chunk_name"] = m_ChunkName;
    root["plaintext_mac"] = m_CheckSum;
    root["ciphertext_mac"] = m_CipherTextChecksum;
    root["iv"] = m_Iv;
}

void ChunkInfo::Deserialize(Json::Value& root)
{
    m_ChunkName = root.get("chunk_name", "").asString();
    m_CheckSum = root.get("plaintext_mac", "").asString();
    m_CipherTextChecksum = root.get("ciphertext_mac", "").asString();
    m_Iv = root.get("iv", "").asString();
}

