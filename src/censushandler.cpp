#include "censushandler.h"

#include <deque>
#include "netlib.h"
#include "pagepost.h"
#include "logutils.h"

namespace attic { 

CensusHandler::CensusHandler(const std::string& posts_feed, const AccessToken& at) {
    posts_feed_ = posts_feed;
    access_token_ = at;
    since_time_ = "0";
}

CensusHandler::~CensusHandler() {}

bool CensusHandler::Inquiry(std::deque<FilePost>& out)  {
    QueryTimeline(out);
    if(out.size())
        return true;
    return false;
}

int CensusHandler::QueryTimeline(std::deque<FilePost>& out) {
    int status = ret::A_OK;

    std::cout<<" file list size incoming : " << out.size() << std::endl;
    std::string next_param;
    for(;;) {
        UrlParams params;
        if(next_param.empty()) {
            std::cout<<" since time in param : " << since_time_ << std::endl;
            params.AddValue("types", cnst::g_attic_file_type);
            params.AddValue("since", since_time_);
            params.AddValue("limit", "200");
            params.AddValue("sort_by", "version.received_at");
        }
        else {
            std::cout<<" NEXT PARAM : " << next_param << std::endl;
            params.DeserializeEncodedString(next_param);
            // deserialize next params into UrlParams
        }
        std::cout<< "params : " << params.asString() << std::endl;
        Response resp;
        netlib::HttpGet(posts_feed_,
                        &params,
                        &access_token_,
                        resp);
        if(resp.code == 200) {
            std::cout<< "query timeline result : " << resp.body << std::endl;
            PagePost pp;
            jsn::DeserializeObject(&pp, resp.body);
            std::cout<<" deserialized " << std::endl;
            Json::Value arr(Json::arrayValue);
            jsn::DeserializeJsonValue(arr, pp.data());
            std::cout<<" deserialized " << std::endl;

            std::cout<<" ARR SIZE : " << arr.size() << std::endl;
            Json::ValueIterator itr = arr.begin();
            for(; itr != arr.end(); itr++) {
                FilePost fp;
                if(jsn::DeserializeObject(&fp, *itr))
                    out.push_back(fp);
            }
            
            if(arr.size()) {
                try {
                    if(pp.pages().next().empty()) {
                        since_time_ = out.front().version()->received_at; // newest one
                        std::cout<<" setting since time : " << since_time_ <<std::endl;
                        break;
                    }
                    else {
                        std::cout<<" NEXT PARAM  : " << pp.pages().next() << std::endl;
                        std::cout<<" size : " << pp.pages().next().size() << std::endl;
                        std::cout<<" assigning ... " << std::endl;
                        next_param.clear();
                        next_param = pp.pages().next();
                        since_time_ = out.front().version()->received_at; // update since time
                        std::cout<<" setting since time : " << since_time_ <<std::endl;
                    }
                }
                catch(std::exception& e) {
                    log::LogException("mak3412", e);
                    break;
                }
            }
            else 
                break;
            std::cout<<" looping ... " << std::endl;
        }
        else {
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("MAP12410a#", resp);
            break;
        }
    }

    return status;
}


} //namespace

