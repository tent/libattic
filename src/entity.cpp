#include "entity.h"

#include <fstream>

#include "utils.h"
#include "errorcodes.h"
#include "netlib.h"
#include "response.h"
#include "accesstoken.h"
#include "clientutils.h"

namespace attic { 

const EntityServer Entity::GetPreferredServer() const {
    EntityServer srv;
    if(server_list_.size()) {
        ServerList::const_iterator itr = server_list_.begin();
        ServerList::const_iterator last_itr = itr;
        int last_preferred = 1000;
        for(;itr != server_list_.end(); itr++) {
            if(atoi((*itr).preference().c_str()) < last_preferred)
                last_itr = itr;
        }
        return (*last_itr);
    }

    return srv;
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
    std::string loaded(pBuf);
    
    if(pBuf) {
        delete[] pBuf;                                                                            
        pBuf = 0;                                                                                 
    }                                                                                             
    // Deserialize into self.                                                                     
    jsn::DeserializeObject(this, loaded);                                              
    
    return ret::A_OK;    
}

void Entity::Serialize(Json::Value& root) {
    root["entity"] = entity_;
    
    Json::Value server_arr(Json::arrayValue);
    SerializeServerList(server_arr);
    root["servers"] = server_arr;

    Json::Value prev_arr(Json::arrayValue);
    SerializePreviousEntities(prev_arr);
    root["previous_entities"] = prev_arr;
}

void Entity::SerializeServerList(Json::Value& val) {
    ServerList::iterator itr = server_list_.begin();
    for(;itr!= server_list_.end(); itr++) {
        Json::Value serv(Json::objectValue);
        (*itr).Serialize(serv);
        val.append(serv);
    }
}

void Entity::SerializePreviousEntities(Json::Value& val) {
    jsn::SerializeVector(previous_entities_, val);
}

void Entity::Deserialize(Json::Value& root) {
    entity_ = root.get("entity", "").asString();
    DeserializeServerList(root["servers"]);
    DeserializePreviousEntities(root["previous_entities"]);
}

void Entity::DeserializeServerList(Json::Value& val) {
    Json::ValueIterator itr = val.begin();
    for(;itr!= val.end(); itr++) {
        EntityServer server;
        jsn::DeserializeObject(&server, (*itr));
        server_list_.push_back(server);
    }
}

void Entity::DeserializePreviousEntities(Json::Value& val) {
    jsn::DeserializeObjectValueIntoVector(val, previous_entities_);
}

} //namespace
