#ifndef POSTHANDLER_H_
#define POSTHANDLER_H_
#pragma once

#include <string>

#include "post.h"
#include "envelope.h"
#include "urlparams.h"
#include "accesstoken.h"
#include "response.h"
#include "netlib.h"
#include "connectionhandler.h"

namespace attic { 
/* Post handler can be templated with any class deriving from Post, or any serializable object really
 */


template <class T>
class PostHandlerImpl;
template <class T>
class PostHandlerTearDownImpl;
template <class T>
class PostHandlerCmImpl;

template <class T>
class PostHandler {
public:
    PostHandler(bool tear_down = true);
    PostHandler(const AccessToken& at, bool tear_down = true);
    ~PostHandler();

    // TODO:: add delete method
    
    int Post(const std::string& post_url,
             const UrlParams* params,
             T& post);

    int Put(const std::string& post_url,
            const UrlParams* params,
            T& post);

    int Get(const std::string& post_url,
            const UrlParams* params,
            T& out);

    void Flush() { response_.clear(); }
    const Response& response() const { return response_; }
    T GetReturnPost();
    std::string GetReturnPostAsString();
private:
    bool tear_down_; // Stand alone connection or use connection manager?
    AccessToken* at_;
    Response response_;

    PostHandlerImpl<T>* impl_;
};

// Note* keep the definition of templated classes in the header. Why? Because you will most
//       likely get a linker error if you separate the definition out to a cpp. Since its a 
//       templated class the compiler will instantiate the class when needed, in whatever translation
//       unit. So we have no guarantee that the definition will be in the same translation unit. 
//       And if you think export is a solution, ITS NOT, non standard, and has been rejected
//       several times. Also DO NOT instantiate the template class def at the bottom of the cpp,
//       this is annoying and adds overhead.
//
template <class T>
PostHandler<T>::PostHandler(bool tear_down) {
    impl_ = NULL;
    at_ = NULL;
    tear_down_ = tear_down;
}

template <class T>
PostHandler<T>::PostHandler(const AccessToken& at, bool tear_down) {
    impl_ = NULL;
    at_ = new AccessToken();
    *at_ = at;
    tear_down_ = tear_down;
}

template <class T>
PostHandler<T>::~PostHandler() {
    if(at_) {
        delete at_;
        at_ = NULL;
    }
}

template <class T>
int PostHandler<T>::Post(const std::string& post_url,
                         const UrlParams* params,
                         T& post) {
    int status = ret::A_OK;
    if(!post.version().id().empty()) {
        // We aren't versioning
        Parent parent;
        parent.version = post.version().id();
        post.version().ClearParents();
        post.PushBackParent(parent);
    }

    std::string body;
    jsn::SerializeObject(&post, body);
    netlib::HttpPost(post_url,
                     post.type(),
                     params,
                     body,
                     at_,
                     response_);
    if(response_.code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    
    return status;
}

template <class T>
int PostHandler<T>::Put(const std::string& post_url,
                        const UrlParams* params,
                        T& post) {
    int status = ret::A_OK;
    if(!post.version().id().empty()) {
        Parent parent;
        parent.version = post.version().id();
        post.version().ClearParents();
        post.PushBackParent(parent);
    }

    std::string body;
    jsn::SerializeObject(&post, body);
    netlib::HttpPut(post_url,
                    post.type(),
                    params,
                    body,
                    at_,
                    response_);
    if(response_.code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    
    return status;
}

template <class T>
int PostHandler<T>::Get(const std::string& post_url,
                        const UrlParams* params,
                        T& out) {
    int status = ret::A_OK;
    ConnectionHandler ch;
    ch.HttpGet(post_url,
                    params,
                    at_,
                    response_);

    if(response_.code == 200) {
        Envelope env;
        jsn::DeserializeObject(&env, response_.body);
        post::DeserializePostIntoObject(env.post(), &out);
    }
    else{
        status = ret::A_FAIL_NON_200;
    }
    return status;
}

template <class T>
T PostHandler<T>::GetReturnPost() {
    T post;
    Envelope env;
    jsn::DeserializeObject(&env, response_.body);
    post::DeserializePostIntoObject(env.post(), &post);
    return post;
}

template <class T>
std::string PostHandler<T>::GetReturnPostAsString() {
    T post;
    Envelope env;
    jsn::DeserializeObject(&env, response_.body);
    std::string raw;
    jsn::SerializeObject(&post, raw);
    return raw;
}

template <class T>
class PostHandlerImpl {
public:
    PostHandlerImpl() {
        at_ = NULL;
    }
    virtual ~PostHandlerImpl() {}

    virtual int Post(const std::string& post_url,
                     const UrlParams* params,
                     T& post) = 0;

    virtual int Put(const std::string& post_url,
                    const UrlParams* params,
                    T& post) = 0;

    virtual int Get(const std::string& post_url,
                    const UrlParams* params,
                    T& out) = 0;


    void Flush() { response_.clear(); }
    const Response& response() const { return response_; }

    T GetReturnPost();
    std::string GetReturnPostAsString();

    AccessToken* at() { return &at_; }
    Response* response() { return &response_; }

    void set_at(AccessToken& at) { 
        at_ = at;
    }

private:
    AccessToken at_;
    Response response_;
};

template <class T>
T PostHandlerImpl<T>::GetReturnPost() {
    T post;
    Envelope env;
    jsn::DeserializeObject(&env, response_.body);
    post::DeserializePostIntoObject(env.post(), &post);
    return post;
}

template <class T>
std::string PostHandlerImpl<T>::GetReturnPostAsString() {
    T post;
    Envelope env;
    jsn::DeserializeObject(&env, response_.body);
    std::string raw;
    jsn::SerializeObject(&post, raw);
    return raw;
}

template <class T>
class PostHandlerTearDownImpl : public PostHandlerImpl<T> {
public:
    PostHandlerTearDownImpl() {}
    ~PostHandlerTearDownImpl() {}

    int Post(const std::string& post_url,
             const UrlParams* params,
             T& post);

    int Put(const std::string& post_url,
            const UrlParams* params,
            T& post);

    int Get(const std::string& post_url,
            const UrlParams* params,
            T& out);
};

template <class T>
int PostHandlerTearDownImpl<T>::Post(const std::string& post_url,
                                     const UrlParams* params,
                                     T& post) {
    int status = ret::A_OK;
    if(!post.version().id().empty()) {
        // We aren't versioning
        Parent parent;
        parent.version = post.version().id();
        post.version().ClearParents();
        post.PushBackParent(parent);
    }

    std::string body;
    jsn::SerializeObject(&post, body);
    netlib::HttpPost(post_url,
                     post.type(),
                     params,
                     body,
                     this->template at<T>(),
                     *(this->template response<T>)());
    if(this->template response<T>()->code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    
    return status;
}

template <class T>
int PostHandlerTearDownImpl<T>::Put(const std::string& post_url,
                                    const UrlParams* params,
                                    T& post) {
    int status = ret::A_OK;
    if(!post.version().id().empty()) {
        Parent parent;
        parent.version = post.version().id();
        post.version().ClearParents();
        post.PushBackParent(parent);
    }

    std::string body;
    jsn::SerializeObject(&post, body);
    netlib::HttpPut(post_url,
                    post.type(),
                    params,
                    body,
                    this->template at<T>(),
                    *(this->template response<T>()));
    if(this->template response<T>()->code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    
    return status;
}

template <class T>
int PostHandlerTearDownImpl<T>::Get(const std::string& post_url,
                                    const UrlParams* params,
                                    T& out) {
    int status = ret::A_OK;
    ConnectionHandler ch;
    ch.HttpGet(post_url,
                    params,
                    this->template at<T>(),
                    *(this->template response<T>()));

    if(this->template response<T>()->code == 200) {
        Envelope env;
        jsn::DeserializeObject(&env, this->template response<T>()->body);
        post::DeserializePostIntoObject(env.post(), &out);
    }
    else{
        status = ret::A_FAIL_NON_200;
    }
    return status;
}

template <class T>
class PostHandlerCmImpl : public PostHandlerImpl<T> { 
public:
    PostHandlerCmImpl() {}
    ~PostHandlerCmImpl() {}

    int Post(const std::string& post_url,
             const UrlParams* params,
             T& post);

    int Put(const std::string& post_url,
            const UrlParams* params,
            T& post);

    int Get(const std::string& post_url,
            const UrlParams* params,
            T& out);
};

template <class T>
int PostHandlerCmImpl<T>::Post(const std::string& post_url,
                            const UrlParams* params,
                            T& post) {
    int status = ret::A_OK;

    return status;
}

template <class T>
int PostHandlerCmImpl<T>::Put(const std::string& post_url,
                           const UrlParams* params,
                           T& post) {
    int status = ret::A_OK;

    return status;
}

template <class T>
int PostHandlerCmImpl<T>::Get(const std::string& post_url,
                           const UrlParams* params,
                           T& out) {
    int status = ret::A_OK;

    return status;
}



} // namespace
#endif

