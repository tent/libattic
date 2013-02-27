#include "profile.h"

#include <cbase64.h>
#include "constants.h"
#include "errorcodes.h"

AtticProfileInfo::AtticProfileInfo()
{
    m_Permissions.SetIsPublic(false);
}

AtticProfileInfo::~AtticProfileInfo()
{

}

void AtticProfileInfo::Serialize(Json::Value& root)
{
    root["salt"] = cb64::base64_encode(reinterpret_cast<const unsigned char*>(m_Salt.c_str()), m_Salt.size());
    root["mk"] = cb64::base64_encode(reinterpret_cast<const unsigned char*>(m_MasterKey.c_str()), m_MasterKey.size());
    root["mk_iv"] = cb64::base64_encode(reinterpret_cast<const unsigned char*>(m_Iv.c_str()), m_Iv.size());

    Json::Value perm(Json::objectValue);
    jsn::SerializeObject(&m_Permissions, perm);
    root["permissions"] = perm;
}

void AtticProfileInfo::Deserialize(Json::Value& root)
{
    m_Salt = cb64::base64_decode(root.get("salt", "").asString());
    m_MasterKey = cb64::base64_decode(root.get("mk", "").asString());
    m_Iv = cb64::base64_decode(root.get("mk_iv", "").asString());
    jsn::DeserializeObject(&m_Permissions, root["permissions"]);
}

CoreProfileInfo::CoreProfileInfo()
{
    m_Permissions.SetIsPublic(true);
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
        jsn::SerializeVector(licenses, m_Licenses);
        root["licenses"] = licenses;
    }

    if(m_Servers.size() > 0)
    {
        Json::Value servers;
        jsn::SerializeVector(servers, m_Servers);
        root["servers"] = servers;
    }

    Json::Value perm(Json::objectValue);
    jsn::SerializeObject(&m_Permissions, perm);
    root["permissions"] = perm;
}

void CoreProfileInfo::Deserialize(Json::Value& root)
{
    m_Entity = root.get("entity", "").asString();
    jsn::DeserializeIntoVector(root["licenses"], m_Licenses); 
    jsn::DeserializeIntoVector(root["servers"], m_Servers); 
    jsn::DeserializeObject(&m_Permissions, root["permissions"]);
}

BasicProfileInfo::BasicProfileInfo()
{
    m_Permissions.SetIsPublic(true);
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

    Json::Value perm(Json::objectValue);
    jsn::SerializeObject(&m_Permissions, perm);
    root["permissions"] = perm;
}

void BasicProfileInfo::Deserialize(Json::Value& root)
{
    m_Name = root.get("name", "").asString();
    m_AvatarUrl = root.get("avatar_url", "").asString();
    m_Birthdate = root.get("birthdate", "").asString();
    m_Location = root.get("location", "").asString();
    m_Gender = root.get("gender", "").asString();   
    m_Bio = root.get("bio", "").asString();      
    jsn::DeserializeObject(&m_Permissions, root["permissions"]);
}

Profile::Profile()
{
    m_pAtticInfo = NULL;
    m_pCoreInfo = NULL;
    m_pBasicInfo = NULL;
}

Profile::Profile(const Profile& rhs)
{
    m_pAtticInfo = new AtticProfileInfo();
    m_pCoreInfo = new CoreProfileInfo();
    m_pBasicInfo = new BasicProfileInfo();

    *m_pAtticInfo = *(rhs.m_pAtticInfo);
    *m_pCoreInfo = *(rhs.m_pCoreInfo);
    *m_pBasicInfo = *(rhs.m_pBasicInfo);
}

Profile Profile::operator=(const Profile& rhs)
{
    m_pAtticInfo = new AtticProfileInfo();
    m_pCoreInfo = new CoreProfileInfo();
    m_pBasicInfo = new BasicProfileInfo();

    *m_pAtticInfo = *(rhs.m_pAtticInfo);
    *m_pCoreInfo = *(rhs.m_pCoreInfo);
    *m_pBasicInfo = *(rhs.m_pBasicInfo);

    return *this;
}

Profile::~Profile()
{
    if(m_pAtticInfo)
    {
        delete m_pAtticInfo;
        m_pAtticInfo = NULL; 
    }

    if(m_pCoreInfo)
    {
        delete m_pCoreInfo;
        m_pCoreInfo = NULL;
    }

    if(m_pBasicInfo)
    {
        delete m_pBasicInfo;
        m_pBasicInfo = NULL;
    }
}

void Profile::Serialize(Json::Value& root)
{
    if(m_pAtticInfo)
    {
        Json::Value attic(Json::objectValue);
        jsn::SerializeObject(m_pAtticInfo, attic);
        root[cnst::g_szAtticProfileType] = attic;
    }

    if(m_pCoreInfo)
    {
        Json::Value core(Json::objectValue);
        jsn::SerializeObject(m_pCoreInfo, core);
        root[cnst::g_szCoreProfileType] = core;
    }

    if(m_pBasicInfo)
    {
        Json::Value basic(Json::objectValue);
        jsn::SerializeObject(m_pBasicInfo, basic);
        root[cnst::g_szBasicProfileType] = basic;
    }
}

void Profile::Deserialize(Json::Value& root)
{
    if(!m_pAtticInfo)
        m_pAtticInfo = new AtticProfileInfo();
    jsn::DeserializeObject(m_pAtticInfo, root[cnst::g_szAtticProfileType]);

    if(!m_pCoreInfo)
        m_pCoreInfo = new CoreProfileInfo();
    jsn::DeserializeObject(m_pCoreInfo, root[cnst::g_szCoreProfileType]);

    if(!m_pBasicInfo)
        m_pBasicInfo = new BasicProfileInfo();
    jsn::DeserializeObject(m_pBasicInfo, root[cnst::g_szBasicProfileType]);
}

int Profile::GetApiRoot(std::string& out)
{
    int status = ret::A_OK;

    if(m_pCoreInfo)
    {
        CoreProfileInfo::ServerList* serverList = m_pCoreInfo->GetServerList();
        CoreProfileInfo::ServerList::iterator itr = serverList->begin();
        for(;itr != serverList->end(); itr++)
        {
            out = (*itr);
            break; // TODO :: this will always get the top most server root
        }

    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

