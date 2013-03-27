#include "tentapp.h"

#include <fstream>
#include "utils.h"

RedirectCode::RedirectCode() {}
RedirectCode::~RedirectCode() {}

void RedirectCode::Serialize(Json::Value& root) {
    root["code"] = m_Code;
    root["token_type"] = m_TokenType;
}

void RedirectCode::Deserialize(Json::Value& root) {
    m_Code = root.get("code", "").asString();
    m_TokenType = root.get("token_type", "").asString();
}

void TentApp::Serialize(Json::Value& root) {
    root["id"] = m_AppID;
    root["name"] = m_AppName;
    root["description"] = m_AppDescription;
    root["url"] = m_AppURL;
    root["icon"] = m_AppIcon;
    
    if(m_Scopes.size() > 0) {
        Json::Value scopes(Json::objectValue); // We want scopes to be an object {}// vs []
        //Json::Value scopes(Json::nullValue);
        jsn::SerializeVectorIntoObjectValue(scopes, m_Scopes);
       // jsn::SerializeVector(scopes, m_Scopes);
        root["scopes"] = scopes;
    }

    if((m_RedirectURIs.size() > 0))
    {
        Json::Value redirecturis;
        jsn::SerializeVector(m_RedirectURIs, redirecturis);
        root["redirect_uris"] = redirecturis;
    }

    root["mac_algorithm"] = m_MacAlgorithm;
    root["mac_key_id"] = m_MacKeyID;
    root["mac_key"] = m_MacKey;

    if(m_Authorizations.size() > 0) {
        Json::Value authorizations;
        jsn::SerializeVector(m_Authorizations, authorizations);
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

    jsn::DeserializeObjectValueIntoVector(root["scopes"], m_Scopes);
    jsn::DeserializeIntoVector(root["redirect_uris"], m_RedirectURIs);
    jsn::DeserializeIntoVector(root["authorizations"], m_Authorizations);
}

ret::eCode TentApp::SaveToFile(const std::string& filepath) {
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

ret::eCode TentApp::LoadFromFile(const std::string& filepath) {
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
