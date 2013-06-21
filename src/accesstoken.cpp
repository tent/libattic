#include "accesstoken.h"

#include <fstream>

#include "utils.h"

namespace attic {

AccessToken::AccessToken() {
    time_offset_ = "0";
}

AccessToken::AccessToken(const AccessToken& rhs) {
    access_token_.clear();
    access_token_.append(rhs.access_token_.c_str(), rhs.access_token_.size());

    hawk_key_.clear();
    hawk_key_.append(rhs.hawk_key_.c_str(), rhs.hawk_key_.size());

    hawk_algorithm_.clear();
    hawk_algorithm_.append(rhs.hawk_algorithm_.c_str(), rhs.hawk_algorithm_.size());

    token_type_.clear();
    token_type_.append(rhs.token_type_.c_str(), rhs.token_type_.size());

    app_id_.clear();
    app_id_.append(rhs.app_id_.c_str(), rhs.app_id_.size());

    time_offset_.clear();
    time_offset_.append(rhs.time_offset_.c_str(), rhs.time_offset_.size());
}

AccessToken::~AccessToken() {}

ret::eCode AccessToken::SaveToFile(const std::string& filepath) {
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

ret::eCode AccessToken::LoadFromFile(const std::string& filepath) {
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

    if(pBuf) {
        delete[] pBuf;
        pBuf = 0;
    }
    
    // Deserialize into self.
    jsn::DeserializeObject(this, loaded);

    return ret::A_OK;
}

void AccessToken::Reset() {
    access_token_.clear();
    hawk_key_.clear();
    hawk_algorithm_.clear();
    token_type_.clear();
    app_id_.clear();
}

void AccessToken::Serialize(Json::Value& root) {
    root["access_token"] = access_token_;
    root["hawk_key"] = hawk_key_;
    root["hawk_algorithm"] = hawk_algorithm_;
    root["token_type"] = token_type_;
    root["app_id"] = app_id_;
}

void AccessToken::Deserialize(Json::Value& root) {
    access_token_ = root.get("access_token", "").asString();
    hawk_key_ = root.get("hawk_key", "").asString();
    hawk_algorithm_ = root.get("hawk_algorithm", "").asString();
    token_type_ = root.get("token_type", "").asString();
    app_id_ = root.get("app_id", app_id_).asString();
}

} // namespace
