#include "urlparams.h"

#include <iostream>

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
            // TODO :: make sure this isn't leaking
            char *pPm = curl_easy_escape(NULL, hold.c_str() , hold.size()); 
            out.append(pPm);
            
            if(*valItr != itr->second.back())
                out.append(",");
        }
    }   

}

// TODO replace this uri encode implementation with something
const char SAFE[256] =
{
/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

std::string UriEncode(const std::string & sSrc)
{
    const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
    const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
    const int SRC_LEN = sSrc.length();
    unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
    unsigned char * pEnd = pStart;
    const unsigned char * const SRC_END = pSrc + SRC_LEN;
    
    for (; pSrc < SRC_END; ++pSrc) {
        if (SAFE[*pSrc])
            *pEnd++ = *pSrc;
        else {
            // escape this char
            *pEnd++ = '%';
            *pEnd++ = DEC2HEX[*pSrc >> 4];
            *pEnd++ = DEC2HEX[*pSrc & 0x0F];
        }
    }
    
    std::string sResult((char *)pStart, (char *)pEnd);
    delete [] pStart;
    return sResult;
}

void UrlParams::SerializeAndEncodeToString(std::string &out) const
{
    out.append("?");

    UrlParamMap::const_iterator itr = m_Values.begin();

    for(;itr != m_Values.end(); itr++) {
        if(itr != m_Values.begin()) {
            out.append("&");
        }

        out.append(itr->first);
        out.append("=");

        std::string hold;
        UrlParam::const_iterator valItr = itr->second.begin();
        for(; valItr != itr->second.end(); valItr++) {
            hold.clear();
            hold.append(*valItr);
            
            UriEncode(hold);
            out += hold;
            if(*valItr != itr->second.back())
                out.append(",");
        }
    }
}


