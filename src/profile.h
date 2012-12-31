
#ifndef PROFILE_H_
#define PROFILE_H_
#pragma once

#include <vector>
#include <string>

#include "jsonserializable.h"

class Permissions : public JsonSerializable
{
public:
    Permissions();
    ~Permissions();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    bool GetIsPublic() const { return m_Public; }

    void SetIsPublic(const bool pub) { m_Public = pub; }

private:
    bool    m_Public;

};

class CoreProfileInfo : public JsonSerializable
{
public:
    CoreProfileInfo();
    ~CoreProfileInfo();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

private:
    typedef std::vector<std::string> ServerList;
    typedef std::vector<std::string> LicenseList;

    std::string m_Entity;     //`json:"entity"`   //The canonical entity identitifier.
    ServerList  m_Licenses;   //`json:"licenses"` //The licenses the entity publishes content under.
    LicenseList m_Servers;    //`json:"servers"`  //The canonical API roots that can be used to interact with the entity.
};

class BasicProfileInfo : public JsonSerializable
{
public:
    BasicProfileInfo();
    ~BasicProfileInfo();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

private:
    std::string     m_Name;      //`json:"name"`       // Name to be displayed publicly.
    std::string     m_AvatarUrl; //`json:"avatar_url"` // URL to avatar to be displayed publicly.
    std::string     m_Birthdate; //`json:"birthdate"`  // Date of birth in one of these formats: YYYY-MM-DD, YYYY-MM-DD
    std::string     m_Location;  //`json:"location"`   // Location to be displayed publicly.
    std::string     m_Gender;    //`json:"gender"`     // Gender to be displayed publicly.
    std::string     m_Bio;       //`json:"bio"`        // Biography/self-description to be displayed publicly.
};

class Profile : public JsonSerializable
{
public:
    Profile();
    ~Profile();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);
private:
    CoreProfileInfo     m_CoreInfo;
    BasicProfileInfo    m_BasicInfo;
};

#endif

