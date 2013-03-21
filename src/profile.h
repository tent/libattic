#ifndef PROFILE_H_
#define PROFILE_H_
#pragma once

#include <vector>
#include <string>

#include "jsonserializable.h"
#include "permissions.h"

class AtticProfileInfo : public JsonSerializable {
public:
    AtticProfileInfo();
    ~AtticProfileInfo();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    void GetSalt(std::string& out) const { out = m_Salt; }
    void GetMasterKey(std::string& out) const { out = m_MasterKey; }
    void GetIv(std::string& out) const { out = m_Iv; }

    void SetSalt(const std::string& salt) { m_Salt = salt; }
    void SetMasterKey(const std::string& mk) { m_MasterKey = mk; }
    void SetIv(const std::string& iv) { m_Iv = iv; }

    bool HasMasterKey() { return !m_MasterKey.empty(); }

    void SetRecoverySalt(const std::string& salt) { m_RecoverySalt = salt; }
    void SetRecoveryMasterKey(const std::string& mk) { m_RecoveryMasterKey = mk; }
    void SetRecoveryIv(const std::string& iv) { m_RecoveryIv = iv; }

    void GetRecoverySalt(std::string& out) const { out = m_RecoverySalt; }
    void GetRecoveryMasterKey(std::string& out) const { out = m_RecoveryMasterKey; }
    void GetRecoveryIv(std::string& out) const { out = m_Iv; }

    void SetQuestionSalt(const std::string& salt) { m_QuestionSalt = salt; }
    void SetQuestionMasterKey(const std::string& mk) { m_QuestionMasterKey = mk; }
    void SetQuestionIv(const std::string& iv) { m_QuestionIv = iv; }

    void SetQuestionOne(const std::string& one) { m_QuestionOne = one; }
    void SetQuestionTwo(const std::string& two) { m_QuestionTwo = two; }
    void SetQuestionThree(const std::string& three) { m_QuestionThree = three; }

    void GetQuestionSalt(std::string& out) const { out = m_QuestionSalt; }
    void GetQuestionMasterKey(std::string& out) const { out = m_QuestionMasterKey; }
    void GetQuestionIv(std::string& out) const { out = m_QuestionIv; }

    void GetQuestionOne(std::string& out) { out = m_QuestionOne; }
    void GetQuestionTwo(std::string& out) { out = m_QuestionTwo; }
    void GetQuestionThree(std::string& out) { out = m_QuestionThree; }
    
private:
    Permissions m_Permissions;  // `json:"permissions"`

    std::string m_MasterKey;    // `json:"mk"`    // The encrypted Master Key
    std::string m_Iv;           // `json:"mk_iv"` // The IV used to encrypt the master key
    std::string m_Salt;         // `json:"salt"`  // The encrypted salt

    // Recovery Section
    std::string m_RecoveryMasterKey; // Encrypted Recovery Master Key
    std::string m_RecoveryIv;        // IV used to encrypt the recovery master key
    std::string m_RecoverySalt;      // Encrypted recovery Salt

    // Optional Question Recovery
    std::string m_QuestionMasterKey;
    std::string m_QuestionIv;
    std::string m_QuestionSalt;
    std::string m_QuestionOne;
    std::string m_QuestionTwo;
    std::string m_QuestionThree;
};

class CoreProfileInfo : public JsonSerializable
{
public:
    typedef std::vector<std::string> ServerList;
    typedef std::vector<std::string> LicenseList;

    CoreProfileInfo();
    ~CoreProfileInfo();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    ServerList* GetServerList() { return &m_Servers; }
    void GetServerList(ServerList& out) const { out = m_Servers; }

    LicenseList* GetLicenseList() { return &m_Licenses; }
    void GetLicenseList(LicenseList& out) const { out = m_Licenses; }


private:
    
    Permissions m_Permissions; // `json:"permissions"
    std::string m_Entity;     //`json:"entity"`   //The canonical entity identitifier.
    LicenseList  m_Licenses;   //`json:"licenses"` //The licenses the entity publishes content under.
    ServerList m_Servers;    //`json:"servers"`  //The canonical API roots that can be used to interact with the entity.
};

class BasicProfileInfo : public JsonSerializable
{
public:
    BasicProfileInfo();
    ~BasicProfileInfo();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

private:
    Permissions     m_Permissions;   //`json:"permissions"
    std::string     m_Name;          //`json:"name"`       // Name to be displayed publicly.
    std::string     m_AvatarUrl;     //`json:"avatar_url"` // URL to avatar to be displayed publicly.
    std::string     m_Birthdate;     //`json:"birthdate"`  // Date of birth in one of these formats: YYYY-MM-DD, YYYY-MM-DD
    std::string     m_Location;      //`json:"location"`   // Location to be displayed publicly.
    std::string     m_Gender;        //`json:"gender"`     // Gender to be displayed publicly.
    std::string     m_Bio;           //`json:"bio"`        // Biography/self-description to be displayed publicly.
};

class Profile : public JsonSerializable
{
public:
    Profile();
    Profile(const Profile& rhs);
    Profile operator=(const Profile& rhs);
    ~Profile();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);
    
    AtticProfileInfo* GetAtticInfo() const { return m_pAtticInfo; }
    CoreProfileInfo* GetCoreInfo() const { return m_pCoreInfo; }
    BasicProfileInfo* GetBasicInfo() const { return m_pBasicInfo; }

    void SetAtticInfo(AtticProfileInfo* pInfo) { m_pAtticInfo = pInfo; }
    void SetCoreProfileInfo(CoreProfileInfo* pInfo) { m_pCoreInfo = pInfo; }
    void SetBasicProfileInfo(BasicProfileInfo* pInfo) { m_pBasicInfo = pInfo; }

    int GetApiRoot(std::string& out);

private:
    AtticProfileInfo*    m_pAtticInfo;
    CoreProfileInfo*     m_pCoreInfo;
    BasicProfileInfo*    m_pBasicInfo;
};

#endif

