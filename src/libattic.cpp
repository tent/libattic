#include "libattic.h"

#include <fstream>
#include <string>

#include "errorcodes.h"
#include "connectionmanager.h"
#include "tentapp.h"
#include "jsonserializable.h"
#include "urlparams.h"
#include "post.h"
#include "filemanager.h"
#include "utils.h"

#include "taskarbiter.h"
#include "pulltask.h"
#include "pushtask.h"


// TODO :: 
// Things to wrap with mutexes
//  - app
//  - filemanager
//  - connectionmanager
//  - access token? (perhaps provide a copy per operation)

// TODO :: introduce queue'ing mechanism, to protect against multiple
//         file operation spam
//         probably map string enum::state
//         on each operation lock file set state
//         return new error codes if file is in the process of being processed.
//
// TODO :: Set up methods to set the chunk size and whatnot

// Constants, (later to be abstracted elsewhere)
static const char* g_szAtticPostType = "https://tent.io/types/post/attic/v0.1.0";
static const char* g_szAppData = "app";
static const char* g_szAuthToken = "at";
static const char* g_szManifest = "manifest";

static TentApp* g_pApp = 0;
static FileManager* g_pFileManager = 0;

static AccessToken g_at;

// Consider making these volatile
static std::string g_WorkingDirectory;
static std::string g_ConfigDirectory;
static std::string g_TempDirectory;

static std::string g_Entity;
static std::string g_AuthorizationURL;

// Local utility functions
static int PostFile(const char* szUrl, const char* szFilePath, FileInfo* fi);
static int PutFile(const char* szUrl, const char* szFilePath, FileInfo* fi);
static ret::eCode DeletePost(const std::string& szPostID);
//////// API start

int TestQuery() 
{
    //g_pFileManager->TestQuery();

}

int InitializeFileManager()
{
    // Construct path
    std::string szFilePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szFilePath);
    szFilePath.append(g_szManifest);

    g_pFileManager = new FileManager(szFilePath, g_ConfigDirectory);
    g_pFileManager->SetTempDirectory(g_TempDirectory);

    if(!g_pFileManager->StartupFileManager())
        return ret::A_FAIL_TO_LOAD_FILE;

    return ret::A_OK;
}

int ShutdownFileManager()
{
    std::cout<<"fm"<<std::endl;
    if(g_pFileManager)
    {
        
        std::cout<<"fm"<<std::endl;
        g_pFileManager->ShutdownFileManager();
        std::cout<<"fm"<<std::endl;

        delete g_pFileManager;
        g_pFileManager = NULL;

        std::cout<<"fm"<<std::endl;
    }

    return ret::A_OK;
}


int ShutdownAppInstance()
{
    std::cout<<"here"<<std::endl;
    if(g_pApp)
    {
    std::cout<<"here"<<std::endl;
        delete g_pApp;
        g_pApp = NULL;

    std::cout<<"here"<<std::endl;
    }
    else
    {
        return ret::A_FAIL_INVALID_PTR;
    }

    std::cout<<"here"<<std::endl;
    ConnectionManager::GetInstance()->Shutdown();

    std::cout<<"here"<<std::endl;
    return ret::A_OK;
}

int StartupAppInstance( const char* szAppName, 
                        const char* szAppDescription, 
                        const char* szUrl, 
                        const char* szIcon, 
                        char* redirectUris[], 
                        unsigned int uriCount, 
                        char* scopes[], 
                        unsigned int scopeCount)
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

    pCm->HttpPost( path, 
                   NULL,
                   serialized,
                   response);

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

    UrlParams val;
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

    g_AuthorizationURL.clear();
    g_AuthorizationURL.append(szApiRoot);

    utils::CheckUrlAndAppendTrailingSlash(g_AuthorizationURL);

    g_AuthorizationURL.append("oauth/authorize");

    std::string params;
    val.SerializeToString(params);
    std::cout<<"PARAMS : " << params << std::endl;

    // TODO:: encode these parameters
    //

    g_AuthorizationURL.append(params);

    return ret::A_OK;
}

const char* GetAuthorizationURL()
{
    return g_AuthorizationURL.c_str();
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
    utils::CheckUrlAndAppendTrailingSlash(path);
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
    pCm->HttpPostWithAuth( path, 
                           NULL,
                           serialized, 
                           response, 
                           g_pApp->GetMacAlgorithm(), 
                           g_pApp->GetMacKeyID(), 
                           g_pApp->GetMacKey(), 
                           false);

    std::cout<< " RESPONSE : " << response << std::endl;

    // Should have an auth token
    // deserialize auth token
    if(!JsonSerializer::DeserializeObject(&g_at, response))
        return ret::A_FAIL_TO_DESERIALIZE_OBJECT;
    // perhaps save it out

    // Construct path
    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAuthToken);
    
    g_at.SaveToFile(std::string(szSavePath));

    return ret::A_OK;
}

int LoadAccessToken()
{
    // Construct path
    std::string szFilePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szFilePath);
    szFilePath.append(g_szAuthToken);

    return g_at.LoadFromFile(szFilePath);
}

int SaveAppToFile()
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAppData);

    return g_pApp->SaveToFile(szSavePath);
}

int LoadAppFromFile()
{
    if(!g_pApp)
        g_pApp = new TentApp();                                                

    // Construct path
    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAppData);

    g_pApp->LoadFromFile(szSavePath);

    std::string buffer;
    JsonSerializer::SerializeObject(g_pApp, buffer);
    //std::cout<<" BUFFER : " << buffer << std::endl;

    return ret::A_OK;
}

int PushFileTask(const char* szFilePath, void (*callback)(int, void*) )
{
    TaskArbiter arb;

    PushTask* t = new PushTask( g_pApp, 
                                g_pFileManager, 
                                ConnectionManager::GetInstance(),
                                g_at,
                                g_Entity,
                                szFilePath,
                                g_TempDirectory,
                                callback);

    arb.SpinOffTask(t);

    return ret::A_OK;
}

int DeleteFile(const char* szFileName)
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    FileInfo* fi = g_pFileManager->GetFileInfo(szFileName);

    if(!fi)
        return ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    
    std::string postid;
    fi->GetPostID(postid);

    ret::eCode status = ret::A_OK;
    // Delete post
    status = DeletePost(postid);
    
    // Remove from Manifest
    status = g_pFileManager->RemoveFile(szFileName);

    return status; 
}

static ret::eCode DeletePost(const std::string& szPostID)
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;


    // Modify Post
    std::string posturl = g_Entity;
    posturl += "/tent/posts/";
    posturl += szPostID;

    std::cout<< " DELETE URL : " << posturl << std::endl;

    std::string response;
    ConnectionManager::GetInstance()->HttpDelete( posturl,    
                                                  NULL,
                                                  response,  
                                                  g_at.GetMacAlgorithm(), 
                                                  g_at.GetAccessToken(), 
                                                  g_at.GetMacKey(), 
                                                  true);     

    std::cout<<"RESPONSE : " << response << std::endl;
            
    return ret::A_OK;
}

int PullAllFiles()
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    Manifest::EntriesMap* pEntryMap = g_pFileManager->GetManifestEntries();
    Manifest::EntriesMap::iterator itr = pEntryMap->begin();

    std::string filepath = g_WorkingDirectory;
    utils::CheckUrlAndAppendTrailingSlash(filepath);

    for(;itr != pEntryMap->end(); itr++)
    {
        std::string fn = itr->first;
        PullFileTask((filepath + fn).c_str(), NULL);
    }
    
    return ret::A_OK;
}

int PullFileTask(const char* szFilePath, void (*callback)(int, void*))
{
    TaskArbiter arb;

    PullTask* t = new PullTask( g_pApp, 
                   g_pFileManager, 
                   ConnectionManager::GetInstance(),
                   g_at,
                   g_Entity,
                   szFilePath,
                   g_TempDirectory,
                   callback);

    arb.SpinOffTask(t);

    return ret::A_OK;
}

int GetAtticPostCount() 
{
    if(!g_pApp)
        return -1;

    std::string url = g_Entity;
    // TODO :: make this provider agnostic
    url += "/tent/posts/count";

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(g_szAtticPostType));

    std::string response;
    ConnectionManager::GetInstance()->HttpGetWithAuth( url,
                                                       &params,
                                                       response, 
                                                       g_at.GetMacAlgorithm(), 
                                                       g_at.GetAccessToken(), 
                                                       g_at.GetMacKey(), 
                                                       true);

    std::cout<< "RESPONSE : " << response << std::endl;

    int count = -1;
    count = atoi(response.c_str());

    return count;
}


int SyncAtticPosts()
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;
    
    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;



    int postcount = GetAtticPostCount();

    if(postcount <= 0)
        return ret::A_FAIL_COULD_NOT_FIND_POSTS;

    std::string url = g_Entity;
    url += "/tent/posts";

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(g_szAtticPostType));
    params.AddValue(std::string("limit"), std::string("200"));

    std::string response;
    ConnectionManager::GetInstance()->HttpGetWithAuth( url, 
                                                       &params,
                                                       response, 
                                                       g_at.GetMacAlgorithm(), 
                                                       g_at.GetAccessToken(), 
                                                       g_at.GetMacKey(), 
                                                       true);

    std::cout<< " RESPONSE : " << response << std::endl;

    Json::Value root;
    Json::Reader reader;

    if(!reader.parse(response, root))
        return -1;


    Json::ValueIterator itr = root.begin();

    int count = 0;
    for(;itr != root.end(); itr++)
    {
        Post p;
        //JsonSerializer::DeserializeObject(&p, (*itr).asString());
        
        // Deserialize directly into posts
        p.Deserialize(*itr);
        count++;
        std::cout<<p.GetPostType()<<std::endl;

        // if proper post type
        if(p.GetPostType().compare(g_szAtticPostType) == 0 && p.GetAttachmentCount() > 0)
        {
            // Check Attachment
            Post::AttachmentVec *pVec = p.GetAttachments();
            Post::AttachmentVec::iterator itr = pVec->begin();

            for(;itr != pVec->end(); itr++)
            {
                Attachment* pAtt = (*itr);
                if(pAtt)
                {
                    // Populate Manifest
                    if(!g_pFileManager->FindFileInManifest(pAtt->Name))
                    {

                        std::string path = g_WorkingDirectory;
                        utils::CheckUrlAndAppendTrailingSlash(path);
                        path += pAtt->Name;

                        
                        char szLen[256];
                        memset(szLen, 0, sizeof(char)*256);                        
                        snprintf(szLen, (sizeof(char)*256),  "%u", pAtt->Size);



                       // TODO:: reimplement syncing with proper key stores
                       /* 
                        FileInfo* fi = g_pFileManager->CreateFileInfo( pAtt->Name,
                                                                       path,
                                                                       "",
                                                                       "0",
                                                                       szLen,
                                                                       p.GetID(),
                                                                       "0");

                        g_pFileManager->InsertToManifest(fi);
                        */

                    }
                }
            }
        }
    }

    std::cout<< " COUNT : " << count << std::endl;



    if(postcount > 200)
    {
               // Loop through and gather posts

    }

    PullAllFiles();

    return ret::A_OK;
}

int SaveChanges()
{
    // Use this method to force a system wide save
    
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;
    
    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    ret::eCode status = ret::A_OK;

    if(!g_pFileManager->WriteOutChanges())
        status = ret::A_FAIL_TO_WRITE_OUT_MANIFEST;

    return status;
}

int SetWorkingDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_WorkingDirectory.append(szDir);

    return ret::A_OK;
}

int SetConfigDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_ConfigDirectory.append(szDir);

    return ret::A_OK;
}

int SetTempDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_TempDirectory.append(szDir);

    return ret::A_OK;
}

int SetEntityUrl(const char* szUrl)
{
    if(!szUrl)
        return ret::A_FAIL_INVALID_CSTR;

    g_Entity.append(szUrl);

    return ret::A_OK;
}

const char* GetWorkingDirectory() { return g_WorkingDirectory.c_str(); }
const char* GetConfigDirectory() { return g_ConfigDirectory.c_str(); }
const char* GetEntityUrl() { return g_Entity.c_str(); }

