#ifndef APPUTILS
#define APPUTILS
#pragma once

#include <vector>
#include <string>

#include "tentapp.h"
#include "errorcodes.h"
#include "constants.h"
#include "utils.h"
#include "event.h"
#include "netlib.h"
#include "response.h"
#include "clientutils.h"
#include "entity.h"
#include "libatticutils.h"
#include "urlparams.h"
#include "apppost.h"
#include "posthandler.h"

namespace attic { namespace app {
static int StartupAppInstance(TentApp& app,
                              const std::string& appName,
                              const std::string& appDescription,
                              const std::string& url,
                              const std::string& icon,
                              std::vector<std::string>& uris,
                              std::vector<std::string>& scopes);

static int RegisterApp(const std::string& app_path,
                       TentApp& app, 
                       const std::string& entityurl, 
                       const std::string& configdir);

static void InitAppInstance(const std::string& appName,
                            const std::string& appDescription,
                            const std::string& url,
                            const std::string& icon,
                            const std::string& redirect_uri,
                            TentApp& app);

static int SendAppRegRequest(const std::string& app_path, 
                             TentApp& app,
                             std::string& path_out);

static int RetrieveAppCredentials(const std::string cred_path, TentApp& app);

static int ConstructAppAuthorizationURL(const std::string& path,
                                        TentApp& app,
                                        std::string& urlout);

static int LoadAppFromFile(TentApp& app, const std::string& configdir);
static int SaveAppToFile(TentApp& app, const std::string& configdir);

static int RegisterAttic(const std::string& entityurl,
                         const std::string& name, 
                         const std::string& description, 
                         const std::string& url,
                         const std::string& icon,
                         const std::string& redirect_uri,
                         const std::string& configdir,
                         std::string& url_out) {
    int status = ret::A_OK;
    fs::CreateDirectory(configdir);

    Entity ent;
    status = client::Discover(entityurl, NULL, ent);
    if(status == ret::A_OK) {
        TentApp app;
        InitAppInstance(name, description, url, icon, redirect_uri, app);
        std::string path = ent.GetPreferredServer().new_post();
        std::string cred_path;
        status = SendAppRegRequest(path, app, cred_path);
        if(status == ret::A_OK) {
            std::string c_path = entityurl; 
            utils::CheckUrlAndAppendTrailingSlash(c_path);
            if(cred_path[0] == '/') 
                cred_path.erase(0, 1);
            if(c_path.find("http://") != std::string::npos) {
                // TODO :: this
            }
            //c_path += cred_path;
            status = RetrieveAppCredentials(cred_path, app);
            if(status == ret::A_OK) {
                std::string app_url = ent.GetPreferredServer().oauth_auth();
                ConstructAppAuthorizationURL(app_url, app, url_out);
                status = SaveAppToFile(app, configdir);
            }
        }
    }
    return status;
}

static void InitAppInstance(const std::string& app_name,
                            const std::string& app_description,
                            const std::string& url,
                            const std::string& icon,
                            const std::string& redirect_uri,
                            TentApp& app) {
    app.set_app_name(app_name);
    app.set_app_description(app_description);
    app.set_app_url(url);
    app.set_app_icon(icon);
    app.set_redirect_uri(redirect_uri);
}

static int SendAppRegRequest(const std::string& app_path, 
                             TentApp& app,
                             std::string& path_out) {
    int status = ret::A_OK;
    // Constrcut post body
    AppPost app_post;
    app_post.set_type(cnst::g_app_type);
    app_post.PushBackWriteType(cnst::g_attic_chunk_type);
    app_post.PushBackWriteType(cnst::g_attic_file_type);
    app_post.PushBackWriteType(cnst::g_attic_folder_type);
    app_post.PushBackWriteType(cnst::g_attic_cred_type);
    app_post.PushBackReadType(cnst::g_basic_profile_type);

    app_post.set_name(app.app_name());
    app_post.set_url(app.app_url());
    app_post.set_redirect_uri(app.redirect_uri());
    
    PostHandler<AppPost> ph;
    status = ph.Post(app_path, NULL, app_post);
    std::cout<<" HEADERS : " << ph.response().header.asString() << std::endl;
    std::cout<<" CODE : " << ph.response().code << std::endl;
    std::cout<<" BODY : " << ph.response().body << std::endl;

    if(status == ret::A_OK) {
        Response response = ph.response();
        if(!response.header["Link"].empty()) {
            std::string link_header = response.header["Link"];
            if(link_header.find(cnst::g_cred_rel) != -1){
                client::ExtractLink(link_header, path_out);
            }
        }
        AppPost p = ph.GetReturnPost();
        post::DeserializePostIntoObject(p, &app);
    }

    return status;
}

static int RetrieveAppCredentials(const std::string cred_path, TentApp& app) {
    int status = ret::A_OK;
    std::cout<<" CRED PATH : " << cred_path << std::endl;

    AppPost p;
    PostHandler<AppPost> ph;
    status = ph.Get(cred_path, NULL, p);
    std::cout<<" CODE : " << ph.response().code << std::endl;
    std::cout<<" BODY : " << ph.response().body << std::endl;
           
    if(status == ret::A_OK) {
        app.set_hawk_key_id(p.id());
        Json::Value hawk_key;
        p.get_content("hawk_key", hawk_key);
        app.set_hawk_key(hawk_key.asString());
        Json::Value hawk_algorithm;
        p.get_content("hawk_algorithm", hawk_algorithm);
        app.set_hawk_algorithm(hawk_algorithm.asString());
    }
    else {
        status = ret::A_FAIL_NON_200; 
    }

    return status;
}

int ConstructAppAuthorizationURL(const std::string& path,
                               TentApp& app,
                               std::string& urlout) {
    int status = ret::A_OK;

    std::cout<<"ConstructAppAuthorizationURL : " << path << std::endl;

    UrlParams val;
    val.AddValue(std::string("client_id"), app.app_id());
    
    urlout = path;
    std::string params;
    val.SerializeToString(params);

    urlout.append(params);

    return status;
}           
static int RegisterApp(const std::string& app_path,
                       TentApp& app, 
                       const std::string& entityurl, 
                       const std::string& configdir) {
    int status = ret::A_OK;

    // Constrcut post body
    AppPost app_post;
    app_post.set_type(cnst::g_app_type);
    app_post.PushBackWriteType(cnst::g_attic_chunk_type);
    app_post.PushBackWriteType(cnst::g_attic_file_type);
    app_post.PushBackWriteType(cnst::g_attic_folder_type);
    app_post.PushBackWriteType(cnst::g_attic_cred_type);
    app_post.PushBackReadType(cnst::g_basic_profile_type);

    app_post.set_name(app.app_name());
    app_post.set_url(app.app_url());
    app_post.set_redirect_uri(app.redirect_uri());
    
    std::cout<<" APP PATH : " << app_path << std::endl;
    PostHandler<AppPost> ph;
    status = ph.Post(app_path, NULL, app_post);

    Response response = ph.response();
    std::cout<< " HEADER : " << response.header["Link"] << std::endl;
    std::cout<< " CODE : " << response.code << std::endl;
    std::cout<< " BODY : " << response.body << std::endl;

    if(status == ret::A_OK) {
        if(!response.header["Link"].empty()) {
            std::string cred_link;
            if(response.header["Link"].find("https://tent.io/rels/credentials") != -1)
                client::ExtractMetaLink(response, cred_link);

            PostHandler<TentApp> app_ph;
            status = app_ph.Get(cred_link, NULL, app);
            
            std::cout<< " CODE : " << app_ph.response().code << std::endl;
            std::cout<< " BODY : " << app_ph.response().body << std::endl;
            if(status == ret::A_OK)
                status = SaveAppToFile(app, configdir);
        }
    }
    return status;
}

int RequestUserAuthorizationDetails(const std::string& entityurl,
                                    const std::string& code,
                                    const std::string& configdir) {
    int status = ret::A_OK;

    Entity ent;
    status = client::Discover(entityurl, NULL, ent);
    if(status == ret::A_OK) {
        TentApp app;
        status = LoadAppFromFile(app, configdir);
        if(status == ret::A_OK) {
            // Build redirect code
            RedirectCode rcode;
            rcode.set_code(code);
            rcode.set_token_type(cnst::g_token_type);

            std::string path = ent.GetPreferredServer().oauth_token();
            std::cout<<" TOKEN PATH : " << path << std::endl;

            // serialize RedirectCode
            std::string serialized;
            if(!jsn::SerializeObject(&rcode, serialized))
                return ret::A_FAIL_TO_SERIALIZE_OBJECT;

            std::cout<<" serialized : " << serialized << std::endl;
            std::cout<<" setting app id : " << app.app_id() << std::endl;
            AccessToken auth_at;
            auth_at.set_hawk_algorithm(app.hawk_algorithm());
            auth_at.set_access_token(app.hawk_key_id());
            auth_at.set_hawk_key(app.hawk_key());
            auth_at.set_app_id(app.app_id());

            Response response;
            netlib::HttpPost(path,"", NULL, serialized, &auth_at, response);

            std::cout<<" CODE : " << response.code << std::endl;
            std::cout<<" HEADER : " << response.header.asString() << std::endl;
            std::cout<<" BODY : " << response.body << std::endl;

            if(response.code == 200) {
                AccessToken at;
                status = liba::DeserializeIntoAccessToken(response.body, at);
                if(status == ret::A_OK) {
                    at.set_app_id(app.app_id());
                    status = liba::WriteOutAccessToken(at, configdir);
                }
            }
            else {
                std::ostringstream error;
                error << "Non 200 repsonse" << std::endl;
                error << "Attempted path : " << path << std::endl;
                error << "Code : " << response.code << std::endl;
                error << "Body : " << response.body << std::endl;
                std::string err = error.str();
                log::LogString("NC4218_4RQ", err);
                status = ret::A_FAIL_NON_200;
            }
        }
    }
    return status;
}
static int LoadAppFromFile(TentApp& app, const std::string& configdir) {
    std::string savepath(configdir);
    utils::CheckUrlAndAppendTrailingSlash(savepath);
    savepath.append(cnst::g_szAppDataName);

    int status = app.LoadFromFile(savepath);
    return status;
}

static int SaveAppToFile(TentApp& app, const std::string& configdir) {
    std::string savepath(configdir);
    utils::CheckUrlAndAppendTrailingSlash(savepath);
    savepath.append(cnst::g_szAppDataName);

    int status = app.SaveToFile(savepath);
    return status;
}

}}//namespace
#endif

