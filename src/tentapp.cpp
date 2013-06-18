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

    root["hawk_algorithm"] = hawk_algorithm_;
    root["hawk_key_id"] = hawk_key_id_;
    root["hawk_key"] = hawk_key_;

    root["redirect_uri"] = redirect_uri_;
    

    if(authorizations_.size() > 0) {
        Json::Value authorizations;
        jsn::SerializeVector(authorizations_, authorizations);
        root["authorizations"] = authorizations;
    }

    if(post_types_.size()) {
        // Post Types 
        Json::Value posttype(Json::objectValue);
        Json::Value read_types(Json::arrayValue);
        Json::Value write_types(Json::arrayValue);
        jsn::SerializeVector(post_types_["read"], read_types);
        jsn::SerializeVector(post_types_["write"], write_types);
        posttype["read"] = read_types;
        posttype["write"] = write_types;
        root["post_types"] = posttype;
    }

}

void TentApp::Deserialize(Json::Value& root) {
    if(!root["id"].empty()) app_id_ = root.get("id", "").asString();
    if(!root["name"].empty()) app_name_ = root.get("name", "").asString();
    if(!root["description"].empty()) app_description_ = root.get("description", "").asString();
    if(!root["url"].empty()) app_url_ = root.get("url", "").asString(); 
    if(!root["icon"].empty()) app_icon_ = root.get("icon", "").asString(); 
 
    if(!root["hawk_algorithm"].empty()) hawk_algorithm_ = root.get("hawk_algorithm", "").asString();
    if(!root["hawk_key_id"].empty()) hawk_key_id_ = root.get("hawk_key_id", "").asString();
    if(!root["hawk_key"].empty()) hawk_key_ = root.get("hawk_key", "").asString();

    if(!root["redirect_uri"].empty()) redirect_uri_ = root.get("redirect_uri", "").asString();

    if(!root["authorizations"].empty())
        jsn::DeserializeIntoVector(root["authorizations"], authorizations_);

    // Deserialize Post types
    Json::Value content = root["content"];
    Json::Value posttype(Json::objectValue);
    posttype = content["post_types"];
    jsn::DeserializeIntoVector(posttype["read"], post_types_["read"]);
    jsn::DeserializeIntoVector(posttype["write"], post_types_["write"]);

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
