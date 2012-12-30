#include "profile.h"

#include "constants.h"


CoreProfileInfo::CoreProfileInfo()
{

}

CoreProfileInfo::~CoreProfileInfo()
{

}

void CoreProfileInfo::Serialize(Json::Value& root)
{
    root["entity"] = m_Entity;

    if(m_Licenses.size() > 0)
    {
        Json::Value licenses;
        JsonSerializer::SerializeVector(licenses, m_Licenses);
        root["licenses"] = licenses;
    }

    if(m_Servers.size() > 0)
    {
        Json::Value servers;
        JsonSerializer::SerializeVector(servers, m_Servers);
        root["servers"] = servers;
    }
}

void CoreProfileInfo::Deserialize(Json::Value& root)
{
    m_Entity = root.get("entity", "").asString();
    JsonSerializer::DeserializeIntoVector(root["licenses"], m_Licenses); 
    JsonSerializer::DeserializeIntoVector(root["servers"], m_Servers); 
}

BasicProfileInfo::BasicProfileInfo()
{

}

BasicProfileInfo::~BasicProfileInfo()
{

}

void BasicProfileInfo::Serialize(Json::Value& root)
{
    root["name"] = m_Name;
    root["avatar_url"] = m_AvatarUrl;
    root["birthdate"] = m_Birthdate;
    root["location"] = m_Location;
    root["gender"] = m_Gender;
    root["bio"] = m_Bio;
}

void BasicProfileInfo::Deserialize(Json::Value& root)
{
    m_Name = root.get("name", "").asString();
    m_AvatarUrl = root.get("avatar_url", "").asString();
    m_Birthdate = root.get("birthdate", "").asString();
    m_Location = root.get("location", "").asString();
    m_Gender = root.get("gender", "").asString();   
    m_Bio = root.get("bio", "").asString();      
}

Profile::Profile()
{

}

Profile::~Profile()
{

}

void Profile::Serialize(Json::Value& root)
{
    Json::Value core(Json::objectValue);
    JsonSerializer::SerializeObject(&m_CoreInfo, core);
    root[cnst::g_szCoreProfileType] = core;

    Json::Value basic(Json::objectValue);
    JsonSerializer::SerializeObject(&m_BasicInfo, basic);
    root[cnst::g_szBasicProfileType] = basic;
}

void Profile::Deserialize(Json::Value& root)
{
    JsonSerializer::DeserializeObject(&m_CoreInfo, root[cnst::g_szCoreProfileType]);
    JsonSerializer::DeserializeObject(&m_BasicInfo, root[cnst::g_szBasicProfileType]);
}
