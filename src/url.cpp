#include "url.h"

#include "utils.h"

Url::Url() {}
Url::Url(const std::string& url){
    set_url(url);
}
Url::~Url() {}

void Url::ExtractInfo() {
    std::string complete = url_;

    size_t found =0;
    // Extract Query
    found = complete.find(std::string("?"));
    if (found != std::string::npos) {
        query_  = complete.substr (found); 
        // remove scheme from complete
        complete = complete.substr(0, found);
    }
    // Extrac Scheme
    scheme_.clear();
    found = 0;
    found = complete.find(std::string("://"));
    if (found != std::string::npos) {
        scheme_  = complete.substr (0, found); 
        // remove scheme from complete
        complete = complete.substr(found+3);
    }

    path_.clear();
    found = 0;
    found = complete.find(std::string("/"));
    if (found != std::string::npos) {
        // Extract the path
        path_ = complete.substr(found); // post to end
        complete = complete.substr(0, found);
    }
    
    port_.clear();
    // Check for a Port
    found = 0;
    found = complete.find(std::string(":"));
    if (found != std::string::npos) {
        // Extract the port
        port_ = complete.substr(found+1);
        complete = complete.substr(0, found);
    }

    host_.clear();
    // all that's left is the host
    host_ = complete;
}

void Url::GetRequestURI(std::string &out) {
    out.clear();
    out += path_ + query_;
}
