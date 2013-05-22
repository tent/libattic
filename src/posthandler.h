#ifndef POSTHANDLER_H_
#define POSTHANDLER_H_
#pragma once

#include <string>

#include "post.h"
#include "urlparams.h"
#include "accesstoken.h"
#include "response.h"
#include "netlib.h"

namespace attic { 

template <class T>
class PostHandler {
public:
    PostHandler(const AccessToken& at);
    ~PostHandler();

    int Post(const std::string& post_url,
             const UrlParams* params,
             T& post, 
             Response& response);

    int Put(const std::string& post_url,
            const UrlParams* params,
            T& post, 
            Response& response);

    int Get(const std::string& post_url,
            const UrlParams* params,
            T& out, 
            Response& response);
private:
    AccessToken at_;
};

// Note* keep the definition of templated classes in the header. Why? Because you will most
//       likely get a linker error if you separate the definition out to a cpp. Since its a 
//       templated class the compiler will instantiate the class when needed, in whatever translation
//       unit. So we have no guarantee that the definition will be in the same translation unit. 
//       And if you think export is a solution, ITS NOT, non standard, and has been rejected
//       several times. Also DO NOT instantiate the template class def at the bottom of the cpp,
//       this is annoying and adds overhead.
template <class T>
PostHandler<T>::PostHandler(const AccessToken& at) {
    at_ = at;
}

template <class T>
PostHandler<T>::~PostHandler() {}

template <class T>
int PostHandler<T>::Post(const std::string& post_url,
                         const UrlParams* params,
                         T& post, 
                         Response& response) {

    int status = ret::A_OK;
    std::cout<<" post url : " << post_url << std::endl;

    if(!post.version()->id().empty()) {
        Parent parent;
        parent.version = post.version()->id();
        post.PushBackParent(parent);
    }

    std::string body;
    jsn::SerializeObject(&post, body);
    netlib::HttpPost(post_url,
                     post.type(),
                     params,
                     body,
                     &at_,
                     response);
    if(response.code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    
    return status;
}
template <class T>
int PostHandler<T>::Put(const std::string& post_url,
                        const UrlParams* params,
                        T& post, 
                        Response& response) {

    int status = ret::A_OK;
    std::cout<<" post url : " << post_url << std::endl;

    if(!post.version()->id().empty()) {
        Parent parent;
        parent.version = post.version()->id();
        post.PushBackParent(parent);
    }

    std::string body;
    jsn::SerializeObject(&post, body);
    netlib::HttpPut(post_url,
                    post.type(),
                    params,
                    body,
                    &at_,
                    response);
    if(response.code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    
    return status;
}

template <class T>
int PostHandler<T>::Get(const std::string& post_url,
                        const UrlParams* params,
                        T& out,
                        Response& response) {
    int status = ret::A_OK;
    std::cout<<" Retrieve URL : " << post_url << std::endl;

    netlib::HttpGet(post_url,
                    params,
                    &at_,
                    response);

    if(response.code == 200) {
        jsn::DeserializeObject(&out, response.body);
    }
    else{
        status = ret::A_FAIL_NON_200;
    }
    return status;
}

} // namespace
#endif

