#include "confighandler.h"

#include "filemanager.h"
#include "posthandler.h"

namespace attic { 

ConfigHandler::ConfigHandler(FileManager* fm) {
    file_manager_ = fm;
}

ConfigHandler::~ConfigHandler() {}

bool ConfigHandler::CreateConfigPost(const Entity& ent, const AccessToken* at, Post& out) {
    bool ret = false;
    if(!RetrieveConfigPost(ent, at, out)) {
        PostHandler<Post> ph(at);


    }
    return ret;
}

bool ConfigHandler::UpdateConfigPost(const Entity& ent, const AccessToken* at, Post& post) {
    bool ret = false;
    // Updates Config post with locally stored values
    std::string url;
    if(!post.id().empty()) {
        url = ent.GetPreferredServer().post();
        utils::FindAndReplace(url, "{entity}", ent.entity(), url);
        utils::FindAndReplace(url, "{post}", post.id(), url);
    }
    else { 
        url = ent.GetPreferredServer().posts_feed();
    }
         
    int status = ret::A_OK;
    PostHandler<Post> ph(at);
    if(post.id().empty())
        status = ph.Post(url, NULL, post);
    else
        status = ph.Put(url, NULL, post);
    
    if(status == ret::A_OK)
        ret = true;
    else
        log::LogHttpResponse("ch_1981_081", ph.response());
    return ret;
}

bool ConfigHandler::RetrieveConfigPost(const Entity& ent, const AccessToken* at, Post& out) {
    bool ret = false;
    std::string url = ent.GetPreferredServer().posts_feed();
    // setup params to query for credentials post, there should only ever exist one
    UrlParams params;
    params.AddValue(std::string("types"), std::string(cnst::g_attic_config_type));
    std::string prm;
    params.SerializeToString(prm);

    PostHandler<Post> ph(at);
    Post p;
    int status = ph.Get(url, &params, p);
    if(status == ret::A_OK) {
        Envelope env;
        jsn::DeserializeObject(&env , ph.response().body);
        if(env.posts()->size()) {
            post::DeserializePostIntoObject(env.posts()->front(), &out);
            ret = true;
        }
    }
    return ret;
}

bool ConfigHandler::LoadConfigPost(const Entity& ent, const AccessToken* at, Post& in) {

}

int ConfigHandler::GetConfigPostCount(const Entity& ent, const AccessToken* at) {
    std::string url = ent.GetPreferredServer().posts_feed();
    UrlParams params;
    params.AddValue(std::string("types"), std::string(cnst::g_attic_config_type));
    Response response;
    netlib::HttpHead(url, &params, at, response);
    std::cout<<" code : " << response.code << std::endl;
    std::cout<<" header : " << response.header.asString() << std::endl;
    std::cout<<" body : " << response.body << std::endl;
    int count = -1;
    if(response.code == 200) {
        if(response.header.HasValue("Count"))
        count = atoi(response.header["Count"].c_str());
    }
    else {
        log::LogHttpResponse("41935", response);
    }
    return count;
}

} //namespace
