
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
    root["id"] = m_Id;
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

}

void TentApp::SerializeVector(Json::Value &val, std::vector<std::string> &vec)
{
    std::vector<std::string>::iterator itr = vec.begin();
    for(; itr != vec.end(); itr++)
        val.append(*itr);
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


