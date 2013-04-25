#include "censushandler.h"

#include <deque>
#include "netlib.h"

namespace attic { 

CensusHandler::CensusHandler(const std::string& posts_feed, const AccessToken& at) {
    posts_feed_ = posts_feed;
    access_token_ = at;
}

CensusHandler::~CensusHandler() {}

bool CensusHandler::CensusInquiry()  {
    // Retrieve Census post ( there should only be one, delete otherwise)
    CensusPost p;
    if(GetCensusPost(p)) {
        // compare last known version(s) 
        if(last_known_version_ != p.version()->id) { 
            // if there is a difference, check all files and bump version
            last_known_version_ = p.version()->id;
            return true;
         }
    }
    return false;
}

void CensusHandler::PushVersionBump() {

}

bool CensusHandler::GetCensusPost(CensusPost& out) {
    int post_count = GetCensusPostCount();
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
        Json::Value root;
        Json::Reader reader;
        reader.parse(resp.body, root);

        std::deque<CensusPost> posts;
        Json::ValueIterator itr = root.begin();
        for(; itr != root.end(); itr++) {
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

}

} //namespace

