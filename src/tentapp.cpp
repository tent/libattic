#include "tentapp.h"

#include <fstream>
#include "utils.h"

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
    
    if(scopes_.size() > 0) {
        Json::Value scopes(Json::objectValue); // We want scopes to be an object {}// vs []
        //Json::Value scopes(Json::nullValue);
        jsn::SerializeVectorIntoObjectValue(scopes, scopes_);
       // jsn::SerializeVector(scopes, scopes_);
        root["scopes"] = scopes;
    }

    if((redirect_uris_.size() > 0))
    {
        Json::Value redirecturis;
        jsn::SerializeVector(redirect_uris_, redirecturis);
        root["redirect_uris"] = redirecturis;
    }

    root["mac_algorithm"] = mac_algorithm_;
    root["mac_key_id"] = mac_key_id_;
    root["mac_key"] = mac_key_;

    if(authorizations_.size() > 0) {
        Json::Value authorizations;
        jsn::SerializeVector(authorizations_, authorizations);
        root["authorizations"] = authorizations;
    }
}

void TentApp::Deserialize(Json::Value& root) {
    app_id_ = root.get("id", "").asString();
    app_name_ = root.get("name", "").asString();
    app_description_ = root.get("description", "").asString();
    app_url_ = root.get("url", "").asString(); 
    app_icon_ = root.get("icon", "").asString(); 
 
    mac_algorithm_ = root.get("mac_algorithm", "").asString();
    mac_key_id_ = root.get("mac_key_id", "").asString();
    mac_key_ = root.get("mac_key", "").asString();

    jsn::DeserializeObjectValueIntoVector(root["scopes"], scopes_);
    jsn::DeserializeIntoVector(root["redirect_uris"], redirect_uris_);
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
