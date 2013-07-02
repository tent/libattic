#include "censushandler.h"

#include <deque>
#include "netlib.h"
#include "logutils.h"

#include "connectionhandler.h"

namespace attic { 

CensusHandler::CensusHandler(const std::string& posts_feed, const AccessToken& at) {
    posts_feed_ = posts_feed;
    access_token_ = at;
}

CensusHandler::~CensusHandler() {}

bool CensusHandler::Inquiry(const std::string& fragment, 
                            std::deque<FilePost>& out)  {
    std::deque<Post> post_list;
    int status = QueryTimeline(cnst::g_attic_file_type, fragment, post_list);
    if(post_list.size() && status == ret::A_OK) {
        DeserializePages(post_list, out);
        if(post_list.size()) {
            SetReceivedAt(cnst::g_attic_file_type, 
                          fragment, 
                          post_list.front().version().received_at());
        }
        return true;
    }
    return false;
}

bool CensusHandler::Inquiry(const std::string& fragment, 
                            std::deque<FolderPost>& out) {

    std::deque<Post> post_list;
    int status = QueryTimeline(cnst::g_attic_folder_type, fragment, post_list);
    if(post_list.size() && status == ret::A_OK) {
        DeserializePages(post_list, out);
        if(out.size()) {
            SetReceivedAt(cnst::g_attic_folder_type, 
                          fragment, 
                          post_list.front().version().received_at());
        }
        return true;
    }
    return false;
}

void CensusHandler::SetReceivedAt(const std::string& post_type,
                                  const std::string& fragment, 
                                  const std::string& time) {
    std::string pt = post_type + "#" + fragment;
    fragment_map_[pt] = time; 
    std::cout<<" setting time for : " << pt << " time : " << time << std::endl;
}

// Returns a queue of posts in the order of newest to oldest
int CensusHandler::QueryTimeline(const std::string& post_type, 
                                 const std::string& fragment, 
                                 std::deque<Post>& out) {
    int status = ret::A_OK;
    std::ostringstream timeline_debug;
    timeline_debug << " Timeline Debug " << std::endl;

    std::string pt = post_type + "#" + fragment;

    timeline_debug << "\t post type : " << pt << std::endl;

    if(fragment_map_.find(pt) == fragment_map_.end())
        fragment_map_[pt] = "0"; 

    std::cout<<" file list size incoming : " << out.size() << std::endl;
    std::string next_param;
    for(;;) {
        UrlParams params;
        if(next_param.empty()) {
            std::cout<<" since time in param : " << fragment_map_[pt] << std::endl;
            params.AddValue("types", pt);
            params.AddValue("since", fragment_map_[pt]);
            params.AddValue("limit", "200");
            params.AddValue("sort_by", "version.received_at");
        }
        else {
            //std::cout<<" NEXT PARAM : " << next_param << std::endl;
            params.DeserializeEncodedString(next_param);
            // deserialize next params into UrlParams
        }

        timeline_debug << "\t params : " << params.asString() << std::endl;

        Response resp;
        //ConnectionHandler ch;
        netlib::HttpGet(posts_feed_,
                        &params,
                        &access_token_,
                        resp);
        if(resp.code == 200) {
            timeline_debug << "query timeline result : \n" << resp.body << std::endl;


            Envelope pp;
            jsn::DeserializeObject(&pp, resp.body);

            timeline_debug << "serializing debug : " << jsn::DebugSerializeableObject(&pp) << std::endl;
            // Go through posts
            if(pp.posts()->size()){
                Envelope::PostQueue::iterator itr = pp.posts()->begin(); 
                for(;itr!=pp.posts()->end(); itr++) {
                    out.push_back(*itr);
                }
            }

            try {
                if(pp.pages().next().empty()) {
                    break;
                }
                else {
                    //std::cout<<" NEXT PARAM  : " << pp.pages().next() << std::endl;
                    //std::cout<<" size : " << pp.pages().next().size() << std::endl;
                    //std::cout<<" assigning ... " << std::endl;
                    next_param.clear();
                    next_param = pp.pages().next();
                }
            }
            catch(std::exception& e) {
                log::LogException("mak3412", e);
                break;
            }
            //std::cout<<" looping ... " << std::endl;
        }
        else {
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("MAP12410a#", resp);
            break;
        }
    }

    std::cout<< timeline_debug.str() << std::endl;
    return status;
}

void CensusHandler::DeserializePages(const std::deque<Post>& posts, 
                                     std::deque<FilePost>& out) {
    std::deque<Post>::const_iterator itr = posts.begin();
    for(;itr!= posts.end(); itr++) {
        FilePost fp;
        post::DeserializePostIntoObject(*itr, &fp);
        out.push_back(fp);
    }
}

void CensusHandler::DeserializePages(const std::deque<Post>& posts, 
                                     std::deque<FolderPost>& out) {
    std::deque<Post>::const_iterator itr = posts.begin();
    for(;itr!= posts.end(); itr++) {
        FolderPost fp;
        post::DeserializePostIntoObject(*itr, &fp);
        out.push_back(fp);
    }
}

} //namespace

