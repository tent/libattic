
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
        SerializeVectorIntoObjectValue(scopes, m_Scopes);
        root["scopes"] = scopes;
    }

    if((m_RedirectURIs.size() > 0))
    {
        Json::Value redirecturis;
        SerializeVector(redirecturis, m_RedirectURIs);
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
        SerializeVector(authorizations, m_Authorizations);
        root["authorizations"] = authorizations;
    }
}

void TentApp::Deserialize(Json::Value& root)
{
    m_AppID = root.get("id", "").asString();
    m_AppName = root.get("name", "").asString();
    m_AppDescription = root.get("description", "").asString();
    m_AppURL = root.get("url", "").asString(); 
    m_AppIcon = root.get("icon", "").asString(); 
 
    m_MacAlgorithm = root.get("mac_algorithm", "").asString();
    m_MacKeyID = root.get("mac_key_id", "").asString();
    m_MacKey = root.get("mac_key", "").asString();

    DeserializeObjectValueIntoVector(root["scopes"], m_Scopes);
    DeserializeIntoVector(root["redirect_uris"], m_RedirectURIs);
    DeserializeIntoVector(root["authorizations"], m_Authorizations);
}

void TentApp::SerializeVectorIntoObjectValue(Json::Value &val, std::vector<std::string> &vec)
{
    if(val.isObject())
    {
        std::vector<std::string>::iterator itr = vec.begin();
        for(; itr != vec.end(); itr++)
            val[*itr];
    }
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

void TentApp::DeserializeObjectValueIntoVector(Json::Value &val, std::vector<std::string> &vec)
{
    if(val.isObject())
    {
        vec.clear();
        Json::ValueIterator itr = val.begin();

        for(; itr != val.end(); itr++)
        {
            vec.push_back(itr.key().asString());

        }
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

