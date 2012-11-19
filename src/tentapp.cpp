
#include "tentapp.h"

TentApp::TentApp()
{

}

TentApp::~TentApp()
{

}

void TentApp::RegisterApp()
{

}

void TentApp::Serialize(Json::Value& root)
{
    root["id"] = m_AppID;
    root["name"] = m_AppName;
    root["description"] = m_AppDescription;
    root["url"] = m_AppHomepageURL;
    root["icon"] = m_AppIcon;
    
    Json::Value scopes;
    SerializeVector(scopes, m_Scopes);
    root["scopes"] = scopes;

    Json::Value redirecturis;
    SerializeVector(redirecturis, m_RedirectURIs);
    root["redirect_uris"] = redirecturis;

    root["mac_algorithm"] = m_MacAlgorithm;
    root["mac_key_id"] = m_MacKeyID;
    root["mac_key"] = m_MacKey;

    Json::Value authorizations;
    SerializeVector(authorizations, m_Authorizations);
    root["authorizations"] = authorizations;
}

void TentApp::Deserialize(Json::Value& root)
{
    m_AppID = root.get("id", "").asString();
    m_AppName = root.get("name", "").asString();
    m_AppDescription = root.get("description", "").asString();
    m_AppHomepageURL = root.get("url", "").asString(); 
    m_AppIcon = root.get("icon", "").asString(); 
 
    m_MacAlgorithm = root.get("mac_algorithm", "").asString();
    m_MacKeyID = root.get("mac_key_id", "").asString();
    m_MacKey = root.get("mac_key", "").asString();

    DeserializeIntoVector(root["scopes"], m_Scopes);
    DeserializeIntoVector(root["redirect_uris"], m_RedirectURIs);
    DeserializeIntoVector(root["authorizations"], m_Authorizations);
}

void TentApp::SerializeVector(Json::Value &val, std::vector<std::string> &vec)
{
    std::vector<std::string>::iterator itr = vec.begin();
    for(; itr != vec.end(); itr++)
        val.append(*itr);
}


void TentApp::DeserializeIntoVector(Json::Value &val, std::vector<std::string> &vec)
{
    vec.clear();

    Json::ValueIterator itr = val.begin();
    for(; itr != val.end(); itr++)
    {
        vec.push_back((*itr).asString());
    }
}

void TentApp::RequestAuthorization(std::string& szAppID, RedirectCode& tRedirectCode)
{

}

std::string TentApp::RequestAuthorizationURL()
{

}

std::auto_ptr<AccessToken> TentApp::RequestUserAuthorizationDetails(std::string szCode)
{

}

