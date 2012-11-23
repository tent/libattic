#include "libattic.h"

#include <string>

#include "errorcodes.h"
#include "connectionmanager.h"
#include "tentapp.h"
#include "jsonserializable.h"
#include "urlvalues.h"
#include "post.h"

static TentApp* g_pApp = 0;
static AccessToken g_at;

std::string g_szAuthorizationURL;

// Local utility functions
void CheckUrlAndAppendTrailingSlash(std::string &szString);

//////// API start
//
int StartupAppInstance(const char* szAppName, const char* szAppDescription, const char* szUrl, const char* szIcon, char* redirectUris[], unsigned int uriCount, char* scopes[], unsigned int scopeCount)
{
    g_pApp = new TentApp();                                                

    if(szAppName)
        g_pApp->SetAppName(std::string(szAppName));                    
    if(szAppDescription)
        g_pApp->SetAppDescription(std::string(szAppDescription));   
    if(szIcon)
        g_pApp->SetAppIcon(std::string(szIcon));
    if(szUrl)
        g_pApp->SetAppURL(std::string(szUrl));        

    if(redirectUris)
    {
        for(unsigned int i=0; i < uriCount; i++)
        {
            g_pApp->SetRedirectURI(std::string(redirectUris[i]));
        }
    }

    if(scopes)
    {
        for(unsigned int i=0;i<scopeCount; i++)
        {
            g_pApp->SetScope(std::string(scopes[i]));
        }
    }
    
    return ret::A_OK;
}


int ShutdownAppInstance()
{
    if(g_pApp)
    {
        delete g_pApp;
        g_pApp = 0;
    }
    else
    {
        return ret::A_FAIL_INVALID_PTR;
    }

    ConnectionManager::GetInstance()->Shutdown();

    return ret::A_OK;
}

int RegisterApp(const char* szPostPath)
{
    if(!szPostPath)
        return ret::A_FAIL_INVALID_PTR;

    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string path(szPostPath);
    ConnectionManager* pCm = ConnectionManager::GetInstance();

    // serialize app;
    std::string serialized;
    if(!JsonSerializer::SerializeObject(g_pApp, serialized))
        return ret::A_FAIL_TO_SERIALIZE_OBJECT;

    std::cout << " JSON : " << serialized << std::endl;
    std::string response;
    pCm->HttpPost(path, serialized, response);
    std::cout<< " RESPONSE " << response << std::endl;

    // Deserialize new data into app
    if(!JsonSerializer::DeserializeObject(g_pApp, response))
        return ret::A_FAIL_TO_DESERIALIZE_OBJECT;

    return ret::A_OK;
}

int RequestAppAuthorizationURL(const char* szApiRoot)
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    UrlValues val;
    val.AddValue(std::string("client_id"), g_pApp->GetAppID());

    if(g_pApp->GetRedirectURIs())
    {
        TentApp::RedirectVec* pUris = g_pApp->GetRedirectURIs();
        TentApp::RedirectVec::iterator itr = pUris->begin();

        for(;itr!=pUris->end();itr++)
        {
            val.AddValue(std::string("redirect_uri"), *itr);
        }
    }

    if(g_pApp->GetScopes())
    {
        TentApp::ScopeVec* pScopes = g_pApp->GetScopes();
        TentApp::ScopeVec::iterator itr = pScopes->begin();

        for(;itr!=pScopes->end();itr++)
        {
            val.AddValue(std::string("scope"), *itr);
        }
    }

    val.AddValue("tent_profile_info_types", "all");
    val.AddValue("tent_post_types", "all");
    //val.AddValue("tent_post_types", "https://tent.io/types/posts/status/v0.1.0");

    g_szAuthorizationURL.clear();
    g_szAuthorizationURL.append(szApiRoot);
    CheckUrlAndAppendTrailingSlash(g_szAuthorizationURL);
    g_szAuthorizationURL.append("oauth/authorize");
    std::string params = val.SerializeToString();
    std::cout<<"PARAMS : " << params << std::endl;
    g_szAuthorizationURL.append(params);

    return ret::A_OK;
}

const char* GetAuthorizationURL()
{
    return g_szAuthorizationURL.c_str();
}

void CheckUrlAndAppendTrailingSlash(std::string &szString)
{
    if(szString.empty())
        return;

    if(szString[szString.size()-1] != '/')
        szString.append("/");
}


int RequestUserAuthorizationDetails(const char* szApiRoot, const char* szCode)
{
    if(!szCode)
        return ret::A_FAIL_INVALID_CSTR;

    // Build redirect code
    RedirectCode rcode;
    rcode.SetCode(std::string(szCode));
    rcode.SetTokenType(std::string("mac"));

    std::string path(szApiRoot);
    CheckUrlAndAppendTrailingSlash(path);
    path.append("apps/");
    path.append(g_pApp->GetAppID());
    path.append("/authorizations");

    std::cout<< " PATH : " << path << std::endl;
    // serialize RedirectCode
    std::string serialized;
    if(!JsonSerializer::SerializeObject(&rcode, serialized))
        return ret::A_FAIL_TO_SERIALIZE_OBJECT;

    std::string response;
    ConnectionManager* pCm = ConnectionManager::GetInstance();
    pCm->HttpPostWithAuth(path, serialized, response, g_pApp->GetMacAlgorithm(), g_pApp->GetMacKeyID(), g_pApp->GetMacKey(), false);

    std::cout<< " RESPONSE : " << response << std::endl;

    // Should have an auth token
    // deserialize auth token
    if(!JsonSerializer::DeserializeObject(&g_at, response))
        return ret::A_FAIL_TO_DESERIALIZE_OBJECT;
    // perhaps save it out
    g_at.SaveToFile(std::string("at"));

    return ret::A_OK;
}

int LoadAccessToken(const char* szFilePath)
{
    std::string szSavePath(szFilePath);
    return g_at.LoadFromFile(szFilePath);
}

int SaveAppToFile(const char* szFilePath)
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string szSavePath(szFilePath);
    return g_pApp->SaveToFile(szSavePath);
}


int LoadAppFromFile(const char* szFilePath)
{
    if(!g_pApp)
        g_pApp = new TentApp();                                                

    std::string szSavePath(szFilePath);
    g_pApp->LoadFromFile(szSavePath);

   
    std::string buffer;
    JsonSerializer::SerializeObject(g_pApp, buffer);
    //std::cout<<" BUFFER : " << buffer << std::endl;


    return ret::A_OK;
}

int PostFile(const char* szUrl, const char* szFilePath)
{
    // file path preferably to a chunked file.
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string postType("https://tent.io/types/post/attic/v0.1.0");
    //std::string postType("https://tent.io/types/post/status/v0.1.0");

    // Create a post 
    Post p;
    p.SetType(postType);
    p.SetContent("text", "testing");
    p.SetPermission(std::string("public"), false);

    // Serialize Post
    std::string postBuffer;
    JsonSerializer::SerializeObject(&p, postBuffer);
    std::cout << " POST BUFFER : " << postBuffer << std::endl;
    
    // Multipart post
    std::string url(szUrl);
    std::string filepath(szFilePath);
    std::string response;
    ConnectionManager::GetInstance()->HttpMultipartPost(url, postBuffer, filepath, response, g_at.GetMacAlgorithm(), g_at.GetAccessToken(), g_at.GetMacKey(), true);


    //ConnectionManager::GetInstance()->HttpPostWithAuth(url, postBuffer,response, g_at.GetMacAlgorithm(), g_at.GetAccessToken(), g_at.GetMacKey(), true);
    
    std::cout<<"RESPONSE : " << response << std::endl;


    return ret::A_OK;
}

int GetFile(const char* szUrl, const char* szPostID)
{
    // file path preferably to a chunked file.
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string url(szUrl);
    std::string response;
    ConnectionManager::GetInstance()->HttpGetWithAuth(url, response, g_at.GetMacAlgorithm(), g_at.GetAccessToken(), g_at.GetMacKey(), true);

    std::cout << " RESPONSE : " << response.size() << std::endl;

 
    return ret::A_OK;
}

