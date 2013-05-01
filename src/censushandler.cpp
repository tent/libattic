#include "censushandler.h"

#include <deque>
#include "netlib.h"
#include "pagepost.h"
#include "logutils.h"

namespace attic { 

CensusHandler::CensusHandler() {}
CensusHandler::~CensusHandler() {}

void CensusHandler::Initialize(const std::string& posts_feed, 
                               const std::string& post_path,
                               const AccessToken& at) {
    posts_feed_ = posts_feed;
    post_path_ = post_path;
    access_token_ = at;
    event::RegisterForEvent(this, event::Event::PUSH);
    event::RegisterForEvent(this, event::Event::DELETE);
    event::RegisterForEvent(this, event::Event::RENAME);
}

void CensusHandler::Shutdown() {
    event::UnregisterFromEvent(this, event::Event::PUSH);
    event::UnregisterFromEvent(this, event::Event::DELETE);
    event::UnregisterFromEvent(this, event::Event::RENAME);
}

bool CensusHandler::Inquiry()  {
    // Retrieve Census post ( there should only be one, delete otherwise)
    CensusPost p;
    if(GetCensusPost(p)) {
        std::cout<<" last known version : " << last_known_version_ << std::endl;
        std::cout<<" new version : " << p.version()->id << std::endl;
        // compare last known version(s) 
        if(last_known_version_ != p.version()->id) { 
            // if there is a difference, check all files and bump version
            last_known_version_ = p.version()->id;
            return true;
         }
    }
    return false;
}

int CensusHandler::PushVersionBump() {
    int status = ret::A_OK;
    CensusPost p;
    if(GetCensusPost(p)) {
        std::string posturl, post_id;
        post_id = p.id();
        if(!post_id.empty()) {
            utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

            std::cout<<" BUMP URL : " << posturl<<std::endl;

            Parent parent;
            parent.version = p.version()->id;
            p.PushBackParent(parent);

            std::string post_buffer;
            jsn::SerializeObject(&p, post_buffer);
            
            Response resp;
            status = netlib::HttpPut(posturl,
                                     p.type(),
                                     NULL,
                                     post_buffer,
                                     &access_token_,
                                     resp);
            if(resp.code == 200) {
                //std::cout<<" PUSH RESP : " << std::endl;
                //std::cout<<resp.body<<std::endl;
            }
            else {
                log::LogHttpResponse("CEN3801", resp);
                status = ret::A_FAIL_NON_200;
            }
        }
    }
    return status;
}

bool CensusHandler::GetCensusPost(CensusPost& out) {
    int post_count = GetCensusPostCount();
    std::cout<<" CENSUS POST COUNT : " << post_count << std::endl;
    if(post_count == 0)
        CreateCensusPost(out);
    else if(post_count > 0)
        RetrieveCensusPost(out);
    else 
        return false;
    return true;
}

int CensusHandler::RetrieveCensusPost(CensusPost& out) {
    int status = ret::A_OK;

    UrlParams params;                                                                  
    params.AddValue("types", std::string(cnst::g_attic_census_type));

    Response resp;
    status = netlib::HttpGet(posts_feed_,
                             &params,
                             &access_token_,
                             resp);

    if(resp.code == 200) {
        std::cout<<" RETREVIAL BODY : " << resp.body << std::endl;
        PagePost pp;
        jsn::DeserializeObject(&pp, resp.body);
        Json::Value arr(Json::arrayValue);
        jsn::DeserializeJsonValue(arr, pp.data());
        
        std::deque<CensusPost> posts;
        Json::ValueIterator itr = arr.begin();
        for(; itr != arr.end(); itr++) {
            CensusPost cp;
            if(jsn::DeserializeObject(&cp, *itr))
                    posts.push_back(cp);
        }

        if(posts.size() > 0) {
            // grab the first one (if there are many)
            out = posts[0];
            // delete the rest (if there are many
        }
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int CensusHandler::CreateCensusPost(CensusPost& out) {
    int status = ret::A_OK;

    std::string post_buffer;
    jsn::SerializeObject(&out, post_buffer);

    Response resp;
    status = netlib::HttpPost(posts_feed_,
                              out.type(),
                              NULL,
                              post_buffer,
                              &access_token_,
                              resp);

    if(resp.code == 200) {
        jsn::DeserializeObject(&out, resp.body);
    }

    return status;
}

int CensusHandler::GetCensusPostCount() {
    UrlParams params;                                                                  
    params.AddValue("types", std::string(cnst::g_attic_census_type));

    Response resp;
    netlib::HttpHead(posts_feed_,
                     &params,
                     &access_token_,
                     resp);

    int count = -1;
    if(resp.code == 200) {
        if(resp.header.HasValue("Count"))
            count = atoi(resp.header["Count"].c_str());
    }
 
    return count;
}

void CensusHandler::OnEventRaised(const event::Event& event) {
    std::cout<<" BUMPING VERSION " << std::endl;
    // Event raised, don't really care what, bump the version
    PushVersionBump();
}

} //namespace

