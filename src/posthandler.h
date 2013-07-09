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
#include "posthandlerimpl.h"

namespace attic { 
/* Post handler can be templated with any class deriving from Post, or any serializable object really
 */

template <class T>
class PostHandler {
    PostHandler(const PostHandler& rhs) {}
    PostHandler operator=(const PostHandler& rhs) { return *this; }
public:
    PostHandler(bool tear_down = false);
    PostHandler(const AccessToken at, bool tear_down = false);
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

    int Delete(const std::string& post_url,
               const std::string& post_version,
               const UrlParams* params);

    void Flush() { impl_->Flush(); }
    const Response& response() const { return impl_->response(); }
    T GetReturnPost();
    std::string GetReturnPostAsString();
private:
    bool tear_down_; // Stand alone connection or use connection manager?
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
    tear_down_ = tear_down;
    if(tear_down)
        impl_ = new PostHandlerTearDownImpl<T>();
    else
        impl_ = new PostHandlerCmImpl<T>();
}

template <class T>
PostHandler<T>::PostHandler(const AccessToken at, bool tear_down) {
    tear_down_ = tear_down;
    if(tear_down) {
        impl_ = new PostHandlerTearDownImpl<T>();
    }
    else {
        impl_ = new PostHandlerCmImpl<T>();
    }
    impl_->set_at(at);
}

template <class T>
PostHandler<T>::~PostHandler() {
    if(impl_) {
        delete impl_;
        impl_ = NULL;
    }
    tear_down_ = false;
}

template <class T>
int PostHandler<T>::Post(const std::string& post_url,
                         const UrlParams* params,
                         T& post) {
    return impl_->Post(post_url, params, post);
}

template <class T>
int PostHandler<T>::Put(const std::string& post_url,
                        const UrlParams* params,
                        T& post) {
    return impl_->Put(post_url, params, post);
}

template <class T>
int PostHandler<T>::Get(const std::string& post_url,
                        const UrlParams* params,
                        T& out) {
    return impl_->Get(post_url, params, out); 
}

template <class T>
int PostHandler<T>::Delete(const std::string& post_url,
                           const std::string& post_version,
                           const UrlParams* params) {
    return impl_->Delete(post_url, post_version, params);
}

template <class T>
T PostHandler<T>::GetReturnPost() {
    return impl_->GetReturnPost();
}

template <class T>
std::string PostHandler<T>::GetReturnPostAsString() {
    return impl_->GetReturnPostAsString();
}


} // namespace
#endif

