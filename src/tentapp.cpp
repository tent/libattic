#include "tentapp.h"

#include <fstream>
#include "utils.h"

namespace attic {

void RedirectCode::Serialize(Json::Value& root) {
    root["code"] = code_;
    root["token_type"] = token_type_;
}

void RedirectCode::Deserialize(Json::Value& root) {
    code_ = root.get("code", "").asString();
    token_type_ = root.get("token_type", "").asString();
}

void TentApp::Serialize(Json::Value& root) {
    root["id"] = app_id_;
    root["name"] = app_name_;
    root["description"] = app_description_;
    root["url"] = app_url_;
    root["icon"] = app_icon_;

    root["mac_algorithm"] = mac_algorithm_;
    root["mac_key_id"] = mac_key_id_;
    root["mac_key"] = mac_key_;

    root["redirect_uri"] = redirect_uri_;
    
    if(scopes_.size() > 0) {
        Json::Value scopes(Json::objectValue); // We want scopes to be an object {}// vs []
        jsn::SerializeVectorIntoObjectValue(scopes, scopes_);
        root["scopes"] = scopes;
    }

    if(authorizations_.size() > 0) {
        Json::Value authorizations;
        jsn::SerializeVector(authorizations_, authorizations);
        root["authorizations"] = authorizations;
    }
}

void TentApp::Deserialize(Json::Value& root) {
    if(!root["id"].empty()) app_id_ = root.get("id", "").asString();
    if(!root["name"].empty()) app_name_ = root.get("name", "").asString();
    if(!root["description"].empty()) app_description_ = root.get("description", "").asString();
    if(!root["url"].empty()) app_url_ = root.get("url", "").asString(); 
    if(!root["icon"].empty()) app_icon_ = root.get("icon", "").asString(); 
 
    if(!root["mac_algorithm"].empty()) mac_algorithm_ = root.get("mac_algorithm", "").asString();
    if(!root["mac_key_id"].empty()) mac_key_id_ = root.get("mac_key_id", "").asString();
    if(!root["mac_key"].empty()) mac_key_ = root.get("mac_key", "").asString();

    if(!root["redirect_uri"].empty()) redirect_uri_ = root.get("redirect_uri", "").asString();

    if(!root["scopes"].empty())
        jsn::DeserializeObjectValueIntoVector(root["scopes"], scopes_);
    if(!root["authorizations"].empty())
        jsn::DeserializeIntoVector(root["authorizations"], authorizations_);
}

ret::eCode TentApp::SaveToFile(const std::string& filepath) {
    std::ofstream ofs;

    ofs.open(filepath.c_str(), std::ofstream::out | std::ofstream::binary); 

    if(!ofs.is_open())
        return ret::A_FAIL_OPEN_FILE;

    std::string serialized;
    jsn::SerializeObject(this, serialized);

    ofs.write(serialized.c_str(), serialized.size());
    ofs.close();
   
    return ret::A_OK;
}

ret::eCode TentApp::LoadFromFile(const std::string& filepath) {
    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN_FILE;

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
    jsn::DeserializeObject(this, loaded);

    return ret::A_OK;
}


}//namespace
