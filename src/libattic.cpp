#include "libattic.h"

#include <fstream>
#include <string>

#include "errorcodes.h"
#include "connectionmanager.h"
#include "tentapp.h"
#include "jsonserializable.h"
#include "urlvalues.h"
#include "post.h"
#include "filemanager.h"
#include "utils.h"


// Constants, (later to be abstracted elsewhere)
static const char* g_szAtticPostType = "https://tent.io/types/post/attic/v0.1.0";
static const char* g_szAppData = "app";
static const char* g_szAuthToken = "at";
static const char* g_szManifest = "manifest._mn";

static TentApp* g_pApp = 0;
static FileManager* g_pFileManager = 0;

static AccessToken g_at;

std::string g_szWorkingDirectory;
std::string g_szEntity;
std::string g_szAuthorizationURL;


// Local utility functions
void CheckUrlAndAppendTrailingSlash(std::string &szString);
static int PostFile(const char* szUrl, const char* szFilePath, FileInfo* fi);

//////// API start
//

int InitializeFileManager()
{
    // Construct path
    std::string szFilePath(g_szWorkingDirectory);
    CheckUrlAndAppendTrailingSlash(szFilePath);
    szFilePath.append(g_szManifest);

    g_pFileManager = new FileManager(szFilePath, g_szWorkingDirectory);

    if(!g_pFileManager->StartupFileManager())
        return ret::A_FAIL_TO_LOAD_FILE;
    return ret::A_OK;
}

int ShutdownFileManager()
{
    if(g_pFileManager)
    {
        g_pFileManager->ShutdownFileManager();
        delete g_pFileManager;
        g_pFileManager = 0;
    }

    return ret::A_OK;
}

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

    // Construct path
    std::string szSavePath(g_szWorkingDirectory);
    CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAuthToken);
    
    g_at.SaveToFile(std::string(szSavePath));

    return ret::A_OK;
}

int LoadAccessToken()
{
    // Construct path
    std::string szFilePath(g_szWorkingDirectory);
    CheckUrlAndAppendTrailingSlash(szFilePath);
    szFilePath.append(g_szAuthToken);

    return g_at.LoadFromFile(szFilePath);
}

int SaveAppToFile()
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string szSavePath(g_szWorkingDirectory);
    CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAppData);

    return g_pApp->SaveToFile(szSavePath);
}

int LoadAppFromFile()
{
    if(!g_pApp)
        g_pApp = new TentApp();                                                

    // Construct path
    std::string szSavePath(g_szWorkingDirectory);
    CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAppData);

    g_pApp->LoadFromFile(szSavePath);

    std::string buffer;
    JsonSerializer::SerializeObject(g_pApp, buffer);
    //std::cout<<" BUFFER : " << buffer << std::endl;

    return ret::A_OK;
}

// utility
static int PostFile(const char* szUrl, const char* szFilePath, FileInfo* fi)
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
    

    // Read in file
    std::string filepath(szFilePath);
    unsigned int size = utils::CheckFileSize(filepath);
    if(!size)
        return ret::A_FAIL_OPEN;
    
    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN;

    char* pData = new char[size];
    memset(pData, 0, (size));

    ifs.read(pData, size);
    ifs.close();

    // Multipart post
    std::string url(szUrl);
    std::string response;

    ConnectionManager::GetInstance()->HttpMultipartPost( url, 
                                                         postBuffer, 
                                                         filepath, 
                                                         response, 
                                                         g_at.GetMacAlgorithm(), 
                                                         g_at.GetAccessToken(), 
                                                         g_at.GetMacKey(), 
                                                         pData,
                                                         size,
                                                         true);
    
    std::cout<<"RESPONSE : " << response << std::endl;

    JsonSerializer::DeserializeObject(&p, response);

    std::string postid = p.GetID();
    if(!postid.empty())
    {
       fi->SetPostID(postid); 
       fi->SetPostVersion(0); // temporary for now, change later
       std::cout << " SIZE : " << p.GetAttachments()->size() << std::endl;
       std::cout << " Name : " << (*p.GetAttachments())[0]->Name << std::endl;
    }

    delete pData;

    return ret::A_OK;
}

int PushFile(const char* szFilePath)
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    std::string filepath(szFilePath);
    std::string fn;
    utils::ExtractFileName(filepath, fn);

    FileInfo* fi = g_pFileManager->GetFileInfo(fn);

    if(!fi)
    {
        int status = 0;
        status = g_pFileManager->IndexFile(filepath);

        if(status != ret::A_OK)
            return status;

        fi = g_pFileManager->GetFileInfo(fn);
    }

    // Check for existing post
    if(fi->GetPostID().empty())
    {
        // New Post
        // Construct post url
        std::string posturl = g_szEntity;
        posturl += "/tent/posts";

        std::cout<< " POST URL : " << posturl << std::endl;

        PostFile(posturl.c_str(), szFilePath, fi);
    }
    else
    {
        // Modify Post

    }

    return ret::A_OK;
}

int GetFile(const char* szUrl, const char* szPostID)
{
    // file path preferably to a chunked file.
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string url(szUrl);
    std::string response;

    ConnectionManager::GetInstance()->HttpGetAttachment(url, response, g_at.GetMacAlgorithm(), g_at.GetAccessToken(), g_at.GetMacKey(), true);

    std::cout << " RESPONSE : " << response << std::endl;

    Post p;
    JsonSerializer::DeserializeObject(&p, response);

    std::cout<< " Attachment size : " << p.GetAttachments()->size() << std::endl;

    std::string a;
    JsonSerializer::SerializeObject(&p, a);
    std::cout << " SERIALIZED : " << a << std::endl;

 
    return ret::A_OK;
}

int GetAtticPostCount() 
{
    if(!g_pApp)
        return -1;

    std::string url = g_szEntity;
    // TODO :: make this provider agnostic
    url += "/tent/posts/count";

    UrlValues val;
    val.AddValue(std::string("post_types"), std::string(g_szAtticPostType));

    url += val.SerializeToString();

    std::string response;
    ConnectionManager::GetInstance()->HttpGetWithAuth(url, response, g_at.GetMacAlgorithm(), g_at.GetAccessToken(), g_at.GetMacKey(), true);

    std::cout<< "RESPONSE : " << response << std::endl;

    return 0;
}


int SetWorkingDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_szWorkingDirectory.append(szDir);

    return ret::A_OK;
}

int SetEntityUrl(const char* szUrl)
{
    if(!szUrl)
        return ret::A_FAIL_INVALID_CSTR;

    g_szEntity.append(szUrl);

    return ret::A_OK;
}

const char* GetWorkingDirectory() { return g_szWorkingDirectory.c_str(); }
const char* GetEntityUrl() { return g_szEntity.c_str(); }

