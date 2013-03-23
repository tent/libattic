#include "entity.h"

#include <fstream>

#include "utils.h"
#include "errorcodes.h"
#include "netlib.h"
#include "response.h"
#include "accesstoken.h"
#include "clientutils.h"

Entity::Entity() {
    m_pActiveProfile = NULL;
}

Entity::~Entity(){}

void Entity::Reset() {
    DeleteProfiles();
    m_ProfileUrls.clear();
    m_EntityUrl.clear();
    m_ApiRoot.clear();
}

void Entity::DeleteProfiles() {
    m_Profiles.clear();
}
 
int Entity::WriteToFile(const std::string& filepath) {
    std::ofstream ofs;
    ofs.open(filepath.c_str(), std::ofstream::out | std::ofstream::binary | std::ios::trunc);

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    std::string serialized;
    jsn::SerializeObject(this, serialized);

    ofs.write(serialized.c_str(), serialized.size());
    ofs.close();

    return ret::A_OK;
}

int Entity::LoadFromFile(const std::string& filepath) {
    std::ifstream ifs;                                                                            
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);                        
                                                                                                  
    if(!ifs.is_open())                                                                            
        return ret::A_FAIL_OPEN_FILE;
                                                                                                  
    unsigned int size = utils::CheckIStreamSize(ifs);                                             
    char* pBuf = new char[size+1];                                                                
    pBuf[size] = '\0';                                                                            
                                                                                                  
    ifs.read(pBuf, size);                                                                         
                                                                                                  
    /*
    // sanity check size and readcount should be the same                                         
    int readcount = ifs.gcount();                                                                 
    if(readcount != size)                                                                         
        std::cout<<"READCOUNT NOT EQUAL TO SIZE\n";                                               
        */
    
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

void Entity::Serialize(Json::Value& root) {
    root["entity_url"] = m_EntityUrl;
    root["api_root"] = m_ApiRoot;

    Json::Value urllist;
    jsn::SerializeVector(urllist, m_ProfileUrls); 
    root["profile_urls"] = urllist;

    std::vector<std::string> profiles;
    ProfileList::iterator itr = m_Profiles.begin();

    std::string buffer;
    for(;itr != m_Profiles.end(); itr++) {
        buffer.clear();
        jsn::SerializeObject(&*itr, buffer);
        profiles.push_back(buffer);
    }

    Json::Value profilelist;
    jsn::SerializeVector(profilelist, profiles);
    root["profile_list"] = profilelist;
}

void Entity::Deserialize(Json::Value& root) {
    m_EntityUrl = root.get("entity_url", "").asString();
    m_ApiRoot = root.get("api_root", "").asString();

    jsn::DeserializeIntoVector(root["profile_urls"], m_ProfileUrls);
    std::vector<std::string> profiles;
    jsn::DeserializeIntoVector(root["profile_list"], profiles);

    std::vector<std::string>::iterator itr = profiles.begin();
    for(;itr != profiles.end(); itr++) {
        Profile p;
        jsn::DeserializeObject(&p, *itr);
        m_Profiles.push_back(p);
    }

    // Set the front profile as active by default
    m_pActiveProfile = GetFrontProfile();
}

bool Entity::HasAtticProfileMasterKey() {
    if(m_pActiveProfile) {
        AtticProfileInfo* pi = m_pActiveProfile->GetAtticInfo();
        if(pi)
            return pi->HasMasterKey();
    }
    return false;
}

int Entity::Discover(const std::string& entityurl, const AccessToken* at) {
    int status = ret::A_OK;
    status = client::Discover(entityurl, at, *this);

    if(status == ret::A_OK) {
        // Grab entity api root etc
        RetrieveProfiles(at);
        
        // Set Api root
        Profile* pProf = GetActiveProfile();
        if(pProf) {
            std::string apiroot;
            pProf->GetApiRoot(apiroot);
            SetApiRoot(apiroot);
            SetEntityUrl(entityurl);
        }
        else {
            status = ret::A_FAIL_INVALID_PTR;
        }
    }

    return status; 

}

void Entity::RetrieveProfiles(const AccessToken* at) {
    unsigned int profcount = GetProfileCount();
    if(profcount) {
        const Entity::UrlList* ProfUrlList = GetProfileUrlList();
        Entity::UrlList::const_iterator itr = ProfUrlList->begin();

        while(itr != ProfUrlList->end()) {
            Response response;
            netlib::HttpGet( *itr, 
                             NULL,
                             at,
                             response);

            /*
            std::cout<< " resp : " << response.body << std::endl;
            std::cout<< " code : " << response.code << std::endl;
            */
 
            if(response.code == 200) {
                // Deserialize into Profile Object
                Profile* pProf = new Profile();
                jsn::DeserializeObject(pProf, response.body);
                
                // Push back into entity
                PushBackProfile(pProf);
            }
            itr++;
        }

        Entity::ProfileList* pProfList = GetProfileList();
        if(pProfList)
            SetActiveProfile(&*pProfList->begin());
   }
}
