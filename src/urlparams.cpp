#include "urlparams.h"

#include "netlib.h"

namespace attic {

void UrlParams::AddValue(const std::string& key, const std::string &value) {
    // Note* adding multiple values to the same key pill push back the value
    values_[key].push_back(value);
}

std::vector<std::string> UrlParams::GetValue(const std::string& key) {
    UrlParam value;
    UrlParamMap::iterator itr = values_.find(key);
    if(itr != values_.end()) {
        value = itr->second;
    }
    return value;
}

bool UrlParams::HasValue(const std::string& key) {
    if(values_.find(key) != values_.end()) 
        return true;
    return false;
}

std::string UrlParams::asString() {
    std::string out;
    SerializeToString(out);
    return out;
}

void UrlParams::SerializeToString(std::string &out) const {
    // Note * will serialize values to http format,
    // appendable to a url
    out.append("?");

    UrlParamMap::const_iterator itr = values_.begin();

    for(;itr != values_.end(); itr++) {
        if(itr != values_.begin()) {
            out.append("&");
        }

        out.append(itr->first);
        out.append("=");

        UrlParam::const_iterator valItr = itr->second.begin();
        for(; valItr != itr->second.end(); valItr++) {
            out.append(*valItr);
            
            if(*valItr != itr->second.back())
                out.append(",");
        }
    }
}

void UrlParams::SerializeAndEncodeToString(std::string &out) const {
    out.append("?");

    UrlParamMap::const_iterator itr = values_.begin();

    for(;itr != values_.end(); itr++) {
        if(itr != values_.begin()) {
            out.append("&");
        }

        out.append(itr->first);
        out.append("=");

        std::string hold;
        UrlParam::const_iterator valItr = itr->second.begin();
        for(; valItr != itr->second.end(); valItr++) {
            hold.clear();
            hold.append(*valItr);
            
            out += netlib::UriEncode(hold);
            
            if(*valItr != itr->second.back())
                out.append(",");
        }
    }
}

void UrlParams::DeserializeEncodedString(const std::string& in) {
    values_.clear();
    std::string decoded;
    decoded = netlib::UriDecode(in);
    std::string local = decoded;
    if(local.size()) {
        if(local[0] == '?') {
            local.erase(0,1);
        }
        utils::split s;
        utils::SplitString(local, '&', s);
        utils::split::iterator itr = s.begin();
        for(; itr != s.end(); itr++) {
            size_t pos = itr->find("=");
            if(pos != std::string::npos) {
                std::string key = (*itr).substr(0, pos);
                std::string value = (*itr).substr(pos+1);
                AddValue(key, value);
            }
        }
    }
}

}//namespace
