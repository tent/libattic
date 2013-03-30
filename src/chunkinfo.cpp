#include "chunkinfo.h"

#include "crypto.h"

namespace attic {

ChunkInfo::ChunkInfo() {}

ChunkInfo::ChunkInfo(const std::string& chunkName, const std::string& checkSum) {
    m_Position = 0;
    m_ChunkName = chunkName;
    m_CheckSum = checkSum;
}

ChunkInfo::~ChunkInfo() {
}

void ChunkInfo::Serialize(Json::Value& root) {
    std::string chunkname, plaintextmac, ciphertextmac, iv, position;
    crypto::Base64EncodeString(m_ChunkName, chunkname);
    crypto::Base64EncodeString(m_PlainTextMac, plaintextmac);
    crypto::Base64EncodeString(m_CipherTextMac, ciphertextmac);
    crypto::Base64EncodeString(m_Iv, iv);
    char pos[256] = {'\0'};
    snprintf(pos, 256, "%u", m_Position);
    crypto::Base64EncodeString(std::string(pos), position);

    root["chunk_name"] = chunkname;
    root["plaintext_mac"] = plaintextmac;
    root["ciphertext_mac"] = ciphertextmac; 
    root["iv"] = iv;
    root["position"] = position;
}

void ChunkInfo::Deserialize(Json::Value& root) {
    std::string chunkname = root.get("chunk_name", "").asString();
    std::string plaintextmac = root.get("plaintext_mac", "").asString();
    std::string ciphertextmac = root.get("ciphertext_mac", "").asString();
    std::string iv = root.get("iv", "").asString();
    std::string position = root.get("position", "").asString();

    crypto::Base64DecodeString(chunkname, m_ChunkName);
    crypto::Base64DecodeString(plaintextmac, m_PlainTextMac);
    crypto::Base64DecodeString(ciphertextmac, m_CipherTextMac);
    crypto::Base64DecodeString(iv, m_Iv);
    std::string decodepos;
    crypto::Base64DecodeString(position, decodepos);
    m_Position = atoi(decodepos.c_str());
}

} //namespace
