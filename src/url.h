
#ifndef URL_H_
#define URL_H_
#pragma once

#include <string>

class Url
{
    void ExtractInfo(); // From url
public:
    Url();
    Url(const std::string& szUrl);
    ~Url();
    
    std::string GetUrl()        { return m_Url; }
    std::string GetScheme()     { return m_Scheme; }
    std::string GetHost()       { return m_Host; }
    std::string GetPath()       { return m_Path; }
    std::string GetPort()       { return m_Port; }
    std::string GetQuery()      { return m_Query; }

    void GetRequestURI(std::string &out); // Similar to that of GO's standard lib

    bool HasUrl()       { return !m_Url.empty(); }
    bool HasScheme()    { return !m_Scheme.empty(); }
    bool HasHost()      { return !m_Host.empty(); }
    bool HasPath()      { return !m_Path.empty(); }
    bool HasPort()      { return !m_Port.empty(); }
    bool HasQuery()     { return !m_Query.empty(); }

    void SetUrl(const std::string &url) { m_Url = url; ExtractInfo(); }
private:
    std::string m_Url;
    std::string m_Scheme;   // ex: http https
    std::string m_Host;     // www.example.com sans scheme
    std::string m_Path;
    std::string m_Port;     // may or may not have a port, if not leave empty
    std::string m_Query;
};

#endif

