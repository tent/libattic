#include "httpheader.h"

namespace attic { 

void HttpHeader::AddValue(const std::string& key, const std::string& value) {
    values_[key] = value;
}

std::string HttpHeader::GetValue(const std::string& key) {
    std::string value;
    HttpHeaderMap::iterator itr = values_.find(key);
    if(itr != values_.end()) {
        value = itr->second;
    }

    return value;
}

void HttpHeader::GetValue(const std::string& key, std::string& out) {
    HttpHeaderMap::iterator itr = values_.find(key);
    if(itr != values_.end()) {
        out = itr->second;
    }
}

bool HttpHeader::HasValue(const std::string& key) {
    HttpHeaderMap::iterator itr = values_.find(key);
    if(itr != values_.end()) {
        if(!itr->second.empty())
            return true;
    }

    return false;
}

void Trim(std::string& in) {
    int pos = in.find("\r\n");
    if(pos != std::string::npos)
        in.erase(pos, 2);
}

void HttpHeader::ParseString(const std::string& in) {
    int left = 0, pos = 0, diff = 0;
    std::string key, value;

    while(pos != std::string::npos) {
        // Find Key
        key.clear();
        pos = in.find(": ", left + 1);
        if(pos == std::string::npos) break;
        //pos -= 1;
        diff = pos - left;
        key = in.substr(left, diff);
        left = pos;
        // Find Value
        value.clear();
        pos = in.find("\n", left + 2);
        if(pos == std::string::npos) break;
        //pos += 2;
        diff = pos - (left+2);
        value = in.substr((left+2), diff);
        left = pos;
        // Push back
           
        Trim(key);
        Trim(value);
        AddValue(key, value);
    }
}

void HttpHeader::ReturnAsString(std::string& out) const {
    HttpHeaderMap::const_iterator itr = values_.begin();
   
    for(;itr!= values_.end(); itr++) {
        out += itr->first;
        out += ": ";
        out += itr->second;
        out += "\r\n";
    }
}

std::string HttpHeader::asString() const {
    std::string out;
    ReturnAsString(out);
    return out;
}

std::string& HttpHeader::operator[](const std::string& index) {
    return values_[index];
}

}//namespace
