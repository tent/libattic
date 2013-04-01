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
            utils::CheckUrlAndRemoveTrailingSlash(c_path);
            c_path += cred_path;
            status = RetrieveAppCredentials(c_path, app);
            if(status == ret::A_OK) {
                std::string app_url = ent.GetPreferredServer().app_auth_request();
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
    
    std::string serialized;
    if(jsn::SerializeObject(&app_post, serialized)) {
        Response response;
        status = netlib::HttpPost(app_path, 
                                  app_post.type(),
                                  NULL,
                                  serialized,
                                  NULL,
                                  response);

        std::cout<<" CODE : " << response.code << std::endl;
        std::cout<<" BODY : " << response.body << std::endl;
        if(response.code == 200) {
            if(!response.header["Link"].empty()) {
                std::string link_header = response.header["Link"];
                if(link_header.find(cnst::g_cred_rel) != -1){
                    client::ExtractLink(link_header, path_out);
                }
            }
            jsn::DeserializeObject(&app, response.body);
        }
        else {
            status = ret::A_FAIL_NON_200;
        }
    }

    return status;
}

static int RetrieveAppCredentials(const std::string cred_path, TentApp& app) {
    int status = ret::A_OK;
    Response resp;
    status = netlib::HttpGet(cred_path, NULL, NULL, resp);
    std::cout<<" CODE : " << resp.code << std::endl;
    std::cout<<" BODY : " << resp.body << std::endl;
           
    if(resp.code == 200) {
        AppPost p;
        jsn::DeserializeObject(&p, resp.body);
        app.set_mac_key_id(p.id());
        Json::Value mac_key;
        p.get_content("mac_key", mac_key);
        app.set_mac_key(mac_key.asString());
        Json::Value mac_algorithm;
        p.get_content("mac_algorithm", mac_algorithm);
        app.set_mac_algorithm(mac_algorithm.asString());
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

    std::string serialized;
    if(jsn::SerializeObject(&app_post, serialized)) {
        Response response;
        status = netlib::HttpPost(app_path, 
                                  app_post.type(),
                                  NULL,
                                  serialized,
                                  NULL,
                                  response);

        std::cout<< " HEADER : " << response.header["Link"] << std::endl;
        std::cout<< " CODE : " << response.code << std::endl;
        std::cout<< " BODY : " << response.body << std::endl;

        if(response.code == 200) {
            if(!response.header["Link"].empty()) {
                std::string cred_link;
                if(response.header["Link"].find("https://tent.io/rels/credentials") != -1)
                    client::ExtractMetaLink(response, cred_link);
                
                Response cred_resp;
                netlib::HttpGet(cred_link, NULL, NULL, cred_resp);
                std::cout<< " CODE : " << cred_resp.code << std::endl;
                std::cout<< " BODY : " << cred_resp.body << std::endl;
            }
            if(jsn::DeserializeObject(&app, response.body)) {
                status = SaveAppToFile(app, configdir);
            }
            else { 
                status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;
            }
        }
        else {
            status = ret::A_FAIL_NON_200;
        }
    }
    else {
        status = ret::A_FAIL_TO_SERIALIZE_OBJECT;
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
        LoadAppFromFile(app, configdir);

        // Build redirect code
        RedirectCode rcode;
        rcode.set_code(code);
        rcode.set_token_type(std::string("mac"));

        std::string path = ent.GetPreferredServer().app_token_request();
        std::cout<<" TOKEN PATH : " << path << std::endl;

        // serialize RedirectCode
        std::string serialized;
        if(!jsn::SerializeObject(&rcode, serialized))
            return ret::A_FAIL_TO_SERIALIZE_OBJECT;

        std::cout<<" serialized : " << serialized << std::endl;

        AccessToken at;
        at.m_MacAlgorithm = app.mac_algorithm();
        at.m_AccessToken = app.mac_key_id();
        at.m_MacKey = app.mac_key();

        Response response;
        netlib::HttpPost(path,"", NULL, serialized, &at, response);

        std::cout<<" CODE : " << response.code << std::endl;
        std::cout<<" BODY : " << response.body << std::endl;

        if(response.code == 200) {
            AccessToken at;
            status = liba::DeserializeIntoAccessToken(response.body, at);
            if(status == ret::A_OK) {
                status = liba::WriteOutAccessToken(at, configdir);
            }
        }
        else {
            std::ostringstream error;
            error << "NC4218_4RQ Non 200 repsonse" << std::endl;
            error << "Code : " << response.code << std::endl;
            error << "Body : " << response.body << std::endl;
            event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);

            status = ret::A_FAIL_NON_200;
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

