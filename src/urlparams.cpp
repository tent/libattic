
#include "urlparams.h"

void UrlParams::AddValue(const std::string& key, const std::string &value)
{
    // Note* adding multiple values to the same key pill push back the value
    m_Values[key].push_back(value);
}

std::vector<std::string> UrlParams::GetValue(const std::string& key)
{
    UrlParam value;
    UrlParamMap::iterator itr = m_Values.find(key);
    if(itr != m_Values.end())
    {
        value = itr->second;
    }
    return value;
}

void UrlParams::SerializeToString(std::string &out) const
{
    // Note * will serialize values to http format,
    // appendable to a url
    out.append("?");

    UrlParamMap::const_iterator itr = m_Values.begin();

    for(;itr != m_Values.end(); itr++)
    {
        if(itr != m_Values.begin())
        {
            out.append("&");
        }

        out.append(itr->first);
        out.append("=");

        UrlParam::const_iterator valItr = itr->second.begin();
        for(; valItr != itr->second.end(); valItr++)
        {
            out.append(*valItr);
            
            if(*valItr != itr->second.back())
                out.append(",");
        }
    }
}

void UrlParams::SerializeAndEncodeToString(CURL* pCurl, std::string &out) const
{
    if(!pCurl)
        return;

    out.append("?");

    UrlParamMap::const_iterator itr = m_Values.begin();

    for(;itr != m_Values.end(); itr++)
    {
        if(itr != m_Values.begin())
        {
            out.append("&");
        }

        out.append(itr->first);
        out.append("=");

        std::string hold;
        UrlParam::const_iterator valItr = itr->second.begin();
        for(; valItr != itr->second.end(); valItr++)
        {
            hold.clear();
            hold.append(*valItr);
            char *pPm = curl_easy_escape(pCurl, hold.c_str() , hold.size()); 
            out.append(pPm);
            
            if(*valItr != itr->second.back())
                out.append(",");
        }
    }

}


