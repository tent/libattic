#include "url.h"

#include "utils.h"


Url::Url()
{


}

Url::Url(const std::string& szUrl)
{
    m_Url = szUrl;

    ExtractInfo();
}

Url::~Url()
{


}

void Url::ExtractInfo()
{

    // copy url locally so we can slice this up
    std::string complete = m_Url;
    // Extract Scheme
    size_t found =0;

    m_Scheme.clear();
    found = complete.find(std::string("://"));
    if (found != std::string::npos)
    {
        m_Scheme  = complete.substr (0, found); 
        // remove scheme from complete
        complete = complete.substr(found+3);
    }

    m_Path.clear();
    found = 0;
    found = complete.find(std::string("/"));
    if (found != std::string::npos)
    {
        // Extract the path
        m_Path = complete.substr(found); // post to end
        complete = complete.substr(0, found);
    }
    
    m_Port.clear();
    // Check for a Port
    found = 0;
    found = complete.find(std::string(":"));
    if (found != std::string::npos)
    {
        // Extract the port
        m_Port = complete.substr(found);
        complete = complete.substr(0, found);
    }

    m_Host.clear();
    // all that's left is the host
    m_Host = complete;
}
    

