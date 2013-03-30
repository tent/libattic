#ifndef URL_H_
#define URL_H_
#pragma once

#include <string>

namespace attic {

class Url {
    void ExtractInfo(); // From url
public:
    Url();
    Url(const std::string& url);
    ~Url();
    
    const std::string& url() const       { return url_; }
    const std::string& scheme() const    { return scheme_; }
    const std::string& host() const      { return host_; }
    const std::string& path() const      { return path_; }
    const std::string& port() const      { return port_; }
    const std::string& query() const     { return query_; }

    void GetRequestURI(std::string &out);

    bool HasUrl()       { return !url_.empty(); }
    bool HasScheme()    { return !scheme_.empty(); }
    bool HasHost()      { return !host_.empty(); }
    bool HasPath()      { return !path_.empty(); }
    bool HasPort()      { return !port_.empty(); }
    bool HasQuery()     { return !query_.empty(); }

    void set_url(const std::string &url) { url_ = url; ExtractInfo(); }
private:
    std::string url_;
    std::string scheme_;   // ex: http https
    std::string host_;     // www.example.com sans scheme
    std::string path_;
    std::string port_;     // may or may not have a port, if not leave empty
    std::string query_;
};

}//namespace
#endif

