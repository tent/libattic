#include "confighandler.h"

#include "posthandler.h"
#include "utils.h"
#include "folderpost.h"

namespace attic { 

ConfigHandler::ConfigHandler(FileManager* fm) {
    file_manager_ = fm;
}

ConfigHandler::~ConfigHandler() {}

bool ConfigHandler::CreateConfigPost(const Entity& ent, const AccessToken* at, ConfigPost& out) {
    bool ret = false;

    if(!RetrieveConfigPost(ent, at, out)) {
        ConfigPost config_post;
        ret = UpdateConfigPost(ent, at, config_post);
        if(ret) {
            RetrieveConfigPost(ent, at, out);
        }
    }
    return ret;
}

bool ConfigHandler::UpdateConfigPost(const Entity& ent, const AccessToken* at, ConfigPost& post) {
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
         
    std::cout<<" access token : " << at->access_token() << std::endl;
    std::cout<<" post type : " << post.type() << std::endl;
    std::cout<<" post id : " << post.id() << std::endl;
    int status = ret::A_OK;
    PostHandler<ConfigPost> ph(*at, false);
    if(post.id().empty()) { 
        std::cout<<" post " << std::endl;
        status = ph.Post(url, NULL, post);
    }
    else {
        std::cout<<" put " << std::endl;
        status = ph.Put(url, NULL, post);

    }
    
    if(status == ret::A_OK)
        ret = true;
    else
        log::LogHttpResponse("ch_1981_081", ph.response());
    return ret;
}

bool ConfigHandler::RetrieveConfigPost(const Entity& ent, const AccessToken* at, ConfigPost& out) {
    bool ret = false;
    std::cout<<" Access token : " << at->access_token() << std::endl;
    std::string url = ent.GetPreferredServer().posts_feed();
    // setup params to query for credentials post, there should only ever exist one
    UrlParams params;
    params.AddValue(std::string("types"), std::string(cnst::g_attic_config_type));
    std::string prm;
    params.SerializeToString(prm);

    PostHandler<ConfigPost> ph(*at, false);
    ConfigPost p;
    int status = ph.Get(url, &params, p);
    if(status == ret::A_OK) {
        Envelope env;
        std::cout<<" Retrieve config post : " << ph.response().body << std::endl;
        jsn::DeserializeObject(&env , ph.response().body);
        if(env.posts()->size()) {
            post::DeserializePostIntoObject(env.posts()->front(), &out);
            ret = true;
        }
    }
    return ret;
}

void ConfigHandler::LoadConfigPost(ConfigPost& in) {
    std::map<std::string, ConfigValue>::iterator itr = in.config_map()->begin();
    for(;itr != in.config_map()->end(); itr++) {
        if(!file_manager_->HasConfigValue((*itr).second.key)) {
            file_manager_->PushConfigValue((*itr).second.type, 
                                           (*itr).second.key,
                                           (*itr).second.value);
        }
    }
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

int ConfigHandler::CreateWorkingDirectory(const std::string& filepath,
                                          const Entity& ent, 
                                          const AccessToken* at) {
    int status = ret::A_OK;
    std::cout<<" creating working directory post " << std::endl;
    // Check if this directory is already linked
    if(!file_manager_->IsDirectoryLinked(filepath)) {
        // Create alias
        std::string postfix;
        utils::GenerateRandomString(postfix, 12);
        std::string name = "~/attic_";
        name += postfix;
        // Create folder post
        Folder folder;
        folder.set_foldername(name);
        folder.set_parent_post_id("root");
        std::string post_id;
        status = CreateFolderPost(folder, ent, at, post_id);
        if(status == ret::A_OK) {
            std::cout<<" created foleder post : " << post_id << std::endl;
            // Insert Into config table;
            file_manager_->AddWorkingDirectory(name, filepath, post_id);
            // Insert folder into folder table;
            Folder f;
            file_manager_->CreateFolderEntry(folder.foldername(), 
                                             folder.folder_post_id(),
                                             folder.parent_post_id(),
                                             f);
            // Update Config Post
            status = AddRootDirToConfigPost(name, post_id, ent, at);
        }
    }

    std::cout<<" create working directory status : " << status << std::endl;
    return status;
}

int ConfigHandler::AddRootDirToConfigPost(const std::string& alias, 
                                          const std::string& postid,
                                          const Entity& ent, 
                                          const AccessToken* at) {
    std::cout<<" Adding Root dir to config " << std::endl;
    int status = ret::A_OK;
    ConfigPost config_post;
    if(RetrieveConfigPost(ent, at, config_post)) {
        config_post.PushBackConfigValue("root",
                                        alias,
                                        postid);
        UpdateConfigPost(ent, at, config_post);  
    }
    return status;
}

int ConfigHandler::CreateFolderPost(Folder& folder, 
                                    const Entity& ent, 
                                    const AccessToken* at,
                                    std::string& id_out) {
    int status = ret::A_OK;
    std::cout<<" Creating folder post " << std::endl;
    std::string posts_feed = ent.GetPreferredServer().posts_feed(); 
    // Create folderpost                                   
    FolderPost fp(folder);                                 
    PostHandler<FolderPost> ph(*at);
    status = ph.Post(posts_feed, NULL, fp);
    if(status != ret::A_OK) {
        log::LogHttpResponse("ch_981lasp151", ph.response());
    }
    else  {
        std::cout << ph.response().body << std::endl;
        FolderPost p = ph.GetReturnPost();
        folder = p.folder();
        id_out = p.id();
    }
    return status;                                         
}


} //namespace
