#include "accesstoken.h"

#include <fstream>

#include "utils.h"

AccessToken::AccessToken()
{

}
AccessToken::~AccessToken()
{

}

ret::eCode AccessToken::SaveToFile(const std::string& filepath)
{
    std::ofstream ofs;

    ofs.open(filepath.c_str(), std::ofstream::out | std::ofstream::binary); 

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    std::string serialized;
    jsn::SerializeObject(this, serialized);

    ofs.write(serialized.c_str(), serialized.size());
    ofs.close();
   
    return ret::A_OK;
}

ret::eCode AccessToken::LoadFromFile(const std::string& filepath)
{
    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    unsigned int size = utils::CheckIStreamSize(ifs);
    char* pBuf = new char[size+1];
    pBuf[size] = '\0';

    ifs.read(pBuf, size);

    // sanity check size and readcount should be the same
    int readcount = ifs.gcount();
    if(readcount != size)
        std::cout<<"READCOUNT NOT EQUAL TO SIZE\n";
    
    std::string loaded(pBuf);

    if(pBuf)
    {
        delete[] pBuf;
        pBuf = 0;
    }
    
    // Deserialize into self.
    jsn::DeserializeObject(this, loaded);

    return ret::A_OK;
}

void AccessToken::Serialize(Json::Value& root)
{
    root["access_token"] = m_AccessToken;
    root["mac_key"] = m_MacKey;
    root["mac_algorithm"] = m_MacAlgorithm;
    root["token_type"] = m_TokenType;
}

void AccessToken::Deserialize(Json::Value& root)
{
    m_AccessToken = root.get("access_token", "").asString();
    m_MacKey = root.get("mac_key", "").asString();
    m_MacAlgorithm = root.get("mac_algorithm", "").asString();
    m_TokenType = root.get("token_type", "").asString();
}

