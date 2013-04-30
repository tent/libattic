#ifndef CENSUSHANDLER_H_
#define CENSUSHANDLER_H_
#pragma once

#include <string>

#include "accesstoken.h"
#include "censuspost.h"
#include "event.h"

namespace attic { 

class CensusHandler : public event::EventListener {
    bool GetCensusPost(CensusPost& out);
    int GetCensusPostCount();
    int RetrieveCensusPost(CensusPost& out);
    int CreateCensusPost(CensusPost& out);


public:
    CensusHandler();
    ~CensusHandler();

    void Initialize(const std::string& posts_feed, 
                    const std::string& post_path,
                    const AccessToken& at);
    void Shutdown();
    bool Inquiry();

    int PushVersionBump();
    void OnEventRaised(const event::Event& event);
private:
    AccessToken access_token_;
    std::string posts_feed_;
    std::string post_path_;
    std::string last_known_version_;
};

} //namespace
#endif

