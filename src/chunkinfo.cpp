#include "chunkinfo.h"

#include <cbase64.h>

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

    root["plaintext_mac"] = cb64::base64_encode(reinterpret_cast<const unsigned char*>(m_PlainTextMac.c_str()), m_PlainTextMac.size());
    root["ciphertext_mac"] = cb64::base64_encode(reinterpret_cast<const unsigned char*>(m_CipherTextMac.c_str()), m_CipherTextMac.size());
    root["iv"] = cb64::base64_encode(reinterpret_cast<const unsigned char*>(m_Iv.c_str()), m_Iv.size());
}

void ChunkInfo::Deserialize(Json::Value& root)
{
    m_ChunkName = root.get("chunk_name", "").asString();

    m_PlainTextMac = cb64::base64_decode(root.get("plaintext_mac", "").asString());
    m_CipherTextMac = cb64::base64_decode(root.get("ciphertext_mac", "").asString());
    m_Iv = cb64::base64_decode(root.get("iv", "").asString());
}

