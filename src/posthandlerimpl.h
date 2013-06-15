#ifndef POSTHANDLERIMPL_H_
#define POSTHANDLERIMPL_H_
#pragma once

#include <string>
#include "post.h"
#include "envelope.h"
#include "response.h"
#include "netlib.h"
#include "connectionhandler.h"

namespace attic { 

template <class T>
class PostHandlerImpl {
public:
    PostHandlerImpl() {}
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
    const Response response() const { return response_; }

    T GetReturnPost();
    std::string GetReturnPostAsString();

    void set_at(const AccessToken at) { 
        at_ = at;
    }

protected:
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
                     &(PostHandlerImpl<T>::at_),
                     PostHandlerImpl<T>::response_);
    if(PostHandlerImpl<T>::response_.code == 200) {
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
                    &(PostHandlerImpl<T>::at_),
                    PostHandlerImpl<T>::response_);
    if(PostHandlerImpl<T>::response_.code == 200) {
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
    netlib::HttpGet(post_url,
                    params,
                    &(PostHandlerImpl<T>::at_),
                    PostHandlerImpl<T>::response_);

    if(PostHandlerImpl<T>::response_.code == 200) {
        Envelope env;
        jsn::DeserializeObject(&env, PostHandlerImpl<T>::response_.body);
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
    if(!post.version().id().empty()) {
        // We aren't versioning
        Parent parent;
        parent.version = post.version().id();
        post.version().ClearParents();
        post.PushBackParent(parent);
    }

    std::string body;
    jsn::SerializeObject(&post, body);
    ConnectionHandler ch;
    ch.HttpPost(post_url,
                post.type(),
                params,
                body,
                &(PostHandlerImpl<T>::at_),
                PostHandlerImpl<T>::response_);
    if(PostHandlerImpl<T>::response_.code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    return status;
}

template <class T>
int PostHandlerCmImpl<T>::Put(const std::string& post_url,
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
    ConnectionHandler ch;
    ch.HttpPut(post_url,
               post.type(),
               params,
               body,
               &(PostHandlerImpl<T>::at_),
               PostHandlerImpl<T>::response_);
    if(PostHandlerImpl<T>::response_.code == 200) {
    }
    else {
        status = ret::A_FAIL_NON_200;
    }
    return status;
}

template <class T>
int PostHandlerCmImpl<T>::Get(const std::string& post_url,
                           const UrlParams* params,
                           T& out) {
    int status = ret::A_OK;
    ConnectionHandler ch;
    ch.HttpGet(post_url,
               params,
               &(PostHandlerImpl<T>::at_),
               PostHandlerImpl<T>::response_);

    if(PostHandlerImpl<T>::response_.code == 200) {
        Envelope env;
        jsn::DeserializeObject(&env, PostHandlerImpl<T>::response_.body);
        post::DeserializePostIntoObject(env.post(), &out);
    }
    else{
        status = ret::A_FAIL_NON_200;
    }
    return status;
}


} // namespace
#endif

