#include "accesstoken.h"

#include <fstream>

#include "utils.h"

namespace attic {
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
