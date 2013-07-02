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
    if(!app_id_.empty()) root["id"] = app_id_;
    if(!app_name_.empty()) root["name"] = app_name_;
    if(!app_description_.empty()) root["description"] = app_description_;
    if(!app_url_.empty()) root["url"] = app_url_;
    if(!app_icon_.empty())   root["icon"] = app_icon_;

    if(!hawk_algorithm_.empty())root["hawk_algorithm"] = hawk_algorithm_;
    if(!hawk_key_id_.empty())root["hawk_key_id"] = hawk_key_id_;
    if(!hawk_key_.empty())root["hawk_key"] = hawk_key_;
    if(!redirect_uri_.empty())root["redirect_uri"] = redirect_uri_;

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
        if(!read_types.empty()) posttype["read"] = read_types;
        if(!write_types.empty()) posttype["write"] = write_types;
        if(!posttype.empty()) root["post_types"] = posttype;
    }

}

void TentApp::Deserialize(Json::Value& root) {
    std::cout<<" APP ID (before) : " << app_id_ << std::endl;
    app_id_ = root.get("id", "").asString();
    std::cout<<" APP ID : " << app_id_ << std::endl;
    app_name_ = root.get("name", "").asString();
    app_description_ = root.get("description", "").asString();
    app_url_ = root.get("url", "").asString(); 
    app_icon_ = root.get("icon", "").asString(); 
 
    hawk_algorithm_ = root.get("hawk_algorithm", "").asString();
    hawk_key_id_ = root.get("hawk_key_id", "").asString();
    hawk_key_ = root.get("hawk_key", "").asString();

    redirect_uri_ = root.get("redirect_uri", "").asString();

    if(!root["authorizations"].isNull())
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
