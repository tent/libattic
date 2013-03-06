#include "httpheader.h"

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{
}

void HttpHeader::AddValue(const std::string& key, const std::string& value)
{
    m_Values[key] = value;
}

std::string HttpHeader::GetValue(const std::string& key)
{
    std::string value;
    HttpHeaderMap::iterator itr = m_Values.find(key);
    if(itr != m_Values.end()) {
        value = itr->second;
    }

    return value;
}

void HttpHeader::GetValue(const std::string& key, std::string& out)
{
    HttpHeaderMap::iterator itr = m_Values.find(key);
    if(itr != m_Values.end()) {
        out = itr->second;
    }
}

bool HttpHeader::ValueExists(const std::string& key)
{
    HttpHeaderMap::iterator itr = m_Values.find(key);
    if(itr != m_Values.end()) {
        if(!itr->second.empty())
            return true;
    }

    return false;
}

