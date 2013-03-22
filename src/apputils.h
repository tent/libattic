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
#include "tentclient.h"
#include "entity.h"
#include "libatticutils.h"
#include "urlparams.h"

namespace app {
static int StartupAppInstance(TentApp& app,
                              const std::string& appName,
                              const std::string& appDescription,
                              const std::string& url,
                              const std::string& icon,
                              std::vector<std::string>& uris,
                              std::vector<std::string>& scopes);

static int RegisterApp(TentApp& app, 
                       const std::string& entityurl, 
                       const std::string& configdir);

static int RequestAppAuthorizationURL(TentApp& app, 
                                      const std::string& entityurl,
                                      std::string& urlout);

static int LoadAppFromFile(TentApp& app, const std::string& configdir);
static int SaveAppToFile(TentApp& app, const std::string& configdir);
static std::string GetEntityApiRoot(const std::string& entityurl);

static int StartupAppInstance(TentApp& app,
                              const std::string& appName,
                              const std::string& appDescription,
                              const std::string& url,
                              const std::string& icon,
                              std::vector<std::string>& uris,
                              std::vector<std::string>& scopes)
{
    int status = ret::A_OK;

    app.SetAppName(appName);
    app.SetAppDescription(appDescription);
    app.SetAppURL(url);
    app.SetAppIcon(icon);

    app.SetRedirectUris(uris);
    app.SetScopes(scopes);

    return status;
}

static int RegisterApp(TentApp& app, 
                       const std::string& entityurl, 
                       const std::string& configdir)
{
    int status = ret::A_OK;

    fs::CreateDirectory(configdir);

    std::string postpath;
    postpath += GetEntityApiRoot(entityurl);
    utils::CheckUrlAndAppendTrailingSlash(postpath);
    postpath += "apps";

    std::string serialized;
    if(jsn::SerializeObject(&app, serialized)) {
        Response response;
        status = netlib::HttpPost( postpath, 
                                   NULL,
                                   serialized,
                                   NULL,
                                   response);

        std::cout<< " CODE : " << response.code << std::endl;
        std::cout<< " BODY : " << response.body << std::endl;
        // Deserialize new data into app
        if(response.code == 200) {
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

int RequestAppAuthorizationURL(TentApp& app, 
                               const std::string& entityurl,
                               std::string& urlout) 
{
    int status = ret::A_OK;

    std::string apiroot;
    apiroot = GetEntityApiRoot(entityurl);

    if(apiroot.empty())
        return ret::A_FAIL_EMPTY_STRING;

    UrlParams val;
    val.AddValue(std::string("client_id"), app.GetAppID());

    if(app.GetRedirectURIs()) {
        TentApp::RedirectVec* pUris = app.GetRedirectURIs();
        TentApp::RedirectVec::iterator itr = pUris->begin();

        for(;itr!=pUris->end();itr++) {
            val.AddValue(std::string("redirect_uri"), *itr);
        }
    }

    if(app.GetScopes()) {
        TentApp::ScopeVec* pScopes = app.GetScopes();
        TentApp::ScopeVec::iterator itr = pScopes->begin();

        for(;itr!=pScopes->end();itr++) {
            val.AddValue(std::string("scope"), *itr);
        }
    }

    val.AddValue("tent_profile_info_types", "all");
    val.AddValue("tent_post_types", "all");
    //val.AddValue("tent_post_types", "https://tent.io/types/posts/status/v0.1.0");

    urlout = apiroot;
    utils::CheckUrlAndAppendTrailingSlash(urlout);
    urlout.append("oauth/authorize");

    std::string params;
    val.SerializeToString(params);

    // TODO:: encode these parameters
    urlout.append(params);

    return status;
}

int RequestUserAuthorizationDetails(TentApp& app,
                                    const std::string& entityurl,
                                    const std::string& code,
                                    const std::string& configdir) 
{
    int status = ret::A_OK;
    LoadAppFromFile(app, configdir);

    std::string apiroot;
    apiroot = GetEntityApiRoot(entityurl);

    if(apiroot.empty())
        return ret::A_FAIL_EMPTY_STRING;

    // Build redirect code
    RedirectCode rcode;
    rcode.SetCode(code);
    rcode.SetTokenType(std::string("mac"));

    std::string path(apiroot);
    utils::CheckUrlAndAppendTrailingSlash(path);
    path.append("apps/");
    path.append(app.GetAppID());
    path.append("/authorizations");

    // serialize RedirectCode
    std::string serialized;
    if(!jsn::SerializeObject(&rcode, serialized))
        return ret::A_FAIL_TO_SERIALIZE_OBJECT;

    Response response;

    AccessToken at;
    at.m_MacAlgorithm = app.GetMacAlgorithm();
    at.m_AccessToken = app.GetMacKeyID();
    at.m_MacKey = app.GetMacKey();

    netlib::HttpPost(path, NULL, serialized, &at, response);

    if(response.code == 200) {
        AccessToken at;
        status = liba::DeserializeIntoAccessToken(response.body, at);
        if(status == ret::A_OK) {
            status = liba::WriteOutAccessToken(at, configdir);
        }
    }
    else {
        std::ostringstream error;
        error << "Non 200 repsonse in RequestUserAuthorizationDetails" << std::endl;
        error << "Code : " << response.code << std::endl;
        error << "Body : " << response.body << std::endl;
        event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);

        status = ret::A_FAIL_NON_200;
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

static std::string GetEntityApiRoot(const std::string& entityurl) {
    Entity out;
    std::string enturl = entityurl;
    utils::CheckUrlAndAppendTrailingSlash(enturl);
    int status = client::Discover(enturl, NULL, out);

    std::string apiroot;
    if(status == ret::A_OK)
        out.GetApiRoot(apiroot);

    return apiroot;
}

}
#endif

