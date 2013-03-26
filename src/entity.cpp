#include "entity.h"

#include <fstream>

#include "utils.h"
#include "errorcodes.h"
#include "netlib.h"
#include "response.h"
#include "accesstoken.h"
#include "clientutils.h"

Entity::Entity() {}
Entity::~Entity(){}

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
    root["entity"] = entity_;

    
    ServerList::iterator itr = server_list_.begin();

    Json::Value server_arr(Json::arrayValue);
    for(;itr!= server_list_.end(); itr++) {
        Json::Value serv(Json::objectValue);
        (*itr).Serialize(serv);
        server_arr.append(serv);
    }
    root["servers"] = server_arr;

    Json::Value prev_arr(Json::arrayValue);
    jsn::SerializeVector(previous_entities_, prev_arr);
    root["previous_entities"] = prev_arr;



}

void Entity::Deserialize(Json::Value& root) {
   
}

