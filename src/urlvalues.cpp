
#include "urlvalues.h"

void UrlValues::AddValue(const std::string& key, const std::string &value)
{
    // Note* adding multiple values to the same key pill push back the value
    m_Values[key].push_back(value);
}

std::vector<std::string> UrlValues::GetValue(const std::string& key)
{
    UrlValue value;
    UrlValueMap::iterator itr = m_Values.find(key);
    if(itr != m_Values.end())
    {
        value = itr->second;
    }
    return value;
}

std::string UrlValues::SerializeToString()
{
    // Note * will serialize values to http format,
    // appendable to a url
    std::string httpValues;
    httpValues.append("?");

    UrlValueMap::iterator itr = m_Values.begin();

    for(;itr != m_Values.end(); itr++)
    {
        if(itr != m_Values.begin())
        {
            httpValues.append("&");
        }

        httpValues.append(itr->first);
        httpValues.append("=");


        UrlValue::iterator valItr = itr->second.begin();
        for(; valItr != itr->second.end(); valItr++)
        {
            httpValues.append(*valItr);
            
            if(*valItr != itr->second.back())
                httpValues.append(",");
        }
    }

    return httpValues;
}

