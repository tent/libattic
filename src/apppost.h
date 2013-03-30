#ifndef APPPOST_H_
#define APPPOST_H_
#pragma once

#include "post.h"

namespace attic {

class AppPost: public Post {
public:
    AppPost(){}
    ~AppPost(){}

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    void PushBackWriteType(const std::string& type);
    void PushBackReadType(const std::string& type);

    const std::string& name() const         { return name_; }
    const std::string& url() const          { return url_; }
    const std::string& redirect_uri() const { return redirect_uri_; }

    void set_name(const std::string& name)          { name_ = name; }
    void set_url(const std::string& url)            { url_ = url; } 
    void set_redirect_uri(const std::string& uri)   { redirect_uri_ = uri; }

private:
    typedef std::map<std::string, std::vector<std::string> > PostTypes;
    PostTypes post_types_;

    std::string name_;
    std::string url_;
    std::string redirect_uri_;
};

} // namespace
#endif

