
#include "tentapp.h"

#include <fstream>

#include "utils.h"


AccessToken::AccessToken()
{

}
AccessToken::~AccessToken()
{

}
ret::eCode AccessToken::SaveToFile(const std::string& szFilePath)
{
    std::ofstream ofs;

    ofs.open(szFilePath.c_str(), std::ofstream::out | std::ofstream::binary); 

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN;

    std::string serialized;
    JsonSerializer::SerializeObject(this, serialized);

    ofs.write(serialized.c_str(), serialized.size());
    ofs.close();
   
    return ret::A_OK;
}

ret::eCode AccessToken::LoadFromFile(const std::string& szFilePath)
{
    std::ifstream ifs;
    ifs.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN;

    unsigned int size = utils::CheckIStreamSize(ifs);
    char* pBuf = new char[size];

    ifs.read(pBuf, size);

    // sanity check size and readcount should be the same
    int readcount = ifs.gcount();
    if(readcount != size)
        std::cout<<"READCOUNT NOT EQUAL TO SIZE\n";
    
    std::string loaded(pBuf);

    if(pBuf)
    {
        delete pBuf;
        pBuf = 0;
    }
    
    // Deserialize into self.
    JsonSerializer::DeserializeObject(this, loaded);

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

RedirectCode::RedirectCode()
{

}

RedirectCode::~RedirectCode()
{

}

void RedirectCode::Serialize(Json::Value& root)
{
    root["code"] = m_Code;
    root["token_type"] = m_TokenType;
}

void RedirectCode::Deserialize(Json::Value& root)
{
    m_Code = root.get("code", "").asString();
    m_TokenType = root.get("token_type", "").asString();
}


TentApp::TentApp()
{

}

TentApp::~TentApp()
{

}

void TentApp::Serialize(Json::Value& root)
{
    if(!m_AppID.empty())
        root["id"] = m_AppID;
    if(!m_AppName.empty())
        root["name"] = m_AppName;
    if(!m_AppDescription.empty())
        root["description"] = m_AppDescription;
    if(!m_AppURL.empty())
        root["url"] = m_AppURL;
    if(!m_AppIcon.empty())
        root["icon"] = m_AppIcon;
    
    if(m_Scopes.size() > 0)
    {
        Json::Value scopes(Json::objectValue); // We want scopes to be an object {}// vs []
        //Json::Value scopes(Json::nullValue);
        JsonSerializer::SerializeVectorIntoObjectValue(scopes, m_Scopes);
       // JsonSerializer::SerializeVector(scopes, m_Scopes);
        root["scopes"] = scopes;
    }

    if((m_RedirectURIs.size() > 0))
    {
        Json::Value redirecturis;
        JsonSerializer::SerializeVector(redirecturis, m_RedirectURIs);
        root["redirect_uris"] = redirecturis;
    }

    if(!m_MacAlgorithm.empty())
        root["mac_algorithm"] = m_MacAlgorithm;
    if(!m_MacKeyID.empty())
        root["mac_key_id"] = m_MacKeyID;
    if(!m_MacKey.empty())
        root["mac_key"] = m_MacKey;

    if(m_Authorizations.size() > 0)
    {
        Json::Value authorizations;
        JsonSerializer::SerializeVector(authorizations, m_Authorizations);
        root["authorizations"] = authorizations;
    }
}

void TentApp::Deserialize(Json::Value& root)
{
    // TODO :: armor this
    m_AppID = root.get("id", "").asString();
    m_AppName = root.get("name", "").asString();
    m_AppDescription = root.get("description", "").asString();
    m_AppURL = root.get("url", "").asString(); 
    m_AppIcon = root.get("icon", "").asString(); 
 
    m_MacAlgorithm = root.get("mac_algorithm", "").asString();
    m_MacKeyID = root.get("mac_key_id", "").asString();
    m_MacKey = root.get("mac_key", "").asString();

    JsonSerializer::DeserializeObjectValueIntoVector(root["scopes"], m_Scopes);
    JsonSerializer::DeserializeIntoVector(root["redirect_uris"], m_RedirectURIs);
    JsonSerializer::DeserializeIntoVector(root["authorizations"], m_Authorizations);
}

ret::eCode TentApp::SaveToFile(const std::string& szFilePath)
{
    std::ofstream ofs;

    ofs.open(szFilePath.c_str(), std::ofstream::out | std::ofstream::binary); 

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN;

    std::string serialized;
    JsonSerializer::SerializeObject(this, serialized);

    ofs.write(serialized.c_str(), serialized.size());
    ofs.close();
   
    return ret::A_OK;
}

ret::eCode TentApp::LoadFromFile(const std::string& szFilePath)
{
    std::ifstream ifs;
    ifs.open(szFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN;

    unsigned int size = utils::CheckIStreamSize(ifs);
    char* pBuf = new char[size];

    ifs.read(pBuf, size);

    // sanity check size and readcount should be the same
    int readcount = ifs.gcount();
    if(readcount != size)
        std::cout<<"READCOUNT NOT EQUAL TO SIZE\n";
    
    std::string loaded(pBuf);

    if(pBuf)
    {
        delete pBuf;
        pBuf = 0;
    }
    
    // Deserialize into self.
    JsonSerializer::DeserializeObject(this, loaded);

    return ret::A_OK;
}
