#include "entity.h"

#include <fstream>

#include "utils.h"
#include "errorcodes.h"

Entity::Entity()
{

}

Entity::~Entity()
{
    ProfileList::iterator itr = m_Profiles.begin();

    while(itr != m_Profiles.end())
    {
        if(*itr)
        {
            // TODO :: step through this and make sure
            //         it does what you think it does
            delete (*itr);
            (*itr) = NULL;
        }
        itr++;
    }

    m_Profiles.clear();
}
 
int Entity::WriteToFile(const std::string& filepath)
{
    std::ofstream ofs;
    ofs.open(filepath.c_str(), std::ofstream::out | std::ofstream::binary);

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN;

    std::string serialized;
    JsonSerializer::SerializeObject(this, serialized);

    ofs.write(serialized.c_str(), serialized.size());
    ofs.close();

    return ret::A_OK;

}

int Entity::LoadFromFile(const std::string& filepath)
{
    std::ifstream ifs;                                                                            
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);                        
                                                                                                  
    if(!ifs.is_open())                                                                            
        return ret::A_FAIL_OPEN;                                                                  
                                                                                                  
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
    JsonSerializer::DeserializeObject(this, loaded);                                              
    
    return ret::A_OK;    
}

void Entity::Serialize(Json::Value& root)
{
    root["entity_url"] = m_EntityUrl;
    root["api_root"] = m_ApiRoot;

    Json::Value urllist;
    JsonSerializer::SerializeVector(urllist, m_ProfileUrls); 
    root["profile_urls"] = urllist;

    Json::Value profilelist;
    JsonSerializer::SerializeObjectVector(profilelist, m_Profiles);
    root["profile_list"] = profilelist;
}

void Entity::Deserialize(Json::Value& root)
{

}


