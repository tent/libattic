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
static const char* g_szManifest = "manifest._mn";

static TentApp* g_pApp = 0;
static FileManager* g_pFileManager = 0;

static AccessToken g_at;

static std::string g_WorkingDirectory;
static std::string g_ConfigDirectory;
static std::string g_TempDirectory;

static std::string g_Entity;
static std::string g_AuthorizationURL;

// Local utility functions
void CheckUrlAndAppendTrailingSlash(std::string &szString);
static int PostFile(const char* szUrl, const char* szFilePath, FileInfo* fi);
static int PutFile(const char* szUrl, const char* szFilePath, FileInfo* fi);
static int GetFileAndWriteOut(const std::string& url, const std::string &filepath);
static int GetFile(const std::string& url, std::string &out);
static ret::eCode DeletePost(const std::string& szPostID);
//////// API start

int InitializeFileManager()
{
    // Construct path
    std::string szFilePath(g_ConfigDirectory);
    CheckUrlAndAppendTrailingSlash(szFilePath);
    szFilePath.append(g_szManifest);

    g_pFileManager = new FileManager(szFilePath, g_ConfigDirectory);
    g_pFileManager->SetTempDirectory(g_TempDirectory);

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

    CheckUrlAndAppendTrailingSlash(g_AuthorizationURL);

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
    CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAuthToken);
    
    g_at.SaveToFile(std::string(szSavePath));

    return ret::A_OK;
}

int LoadAccessToken()
{
    // Construct path
    std::string szFilePath(g_ConfigDirectory);
    CheckUrlAndAppendTrailingSlash(szFilePath);
    szFilePath.append(g_szAuthToken);

    return g_at.LoadFromFile(szFilePath);
}

int SaveAppToFile()
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string szSavePath(g_ConfigDirectory);
    CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAppData);

    return g_pApp->SaveToFile(szSavePath);
}

int LoadAppFromFile()
{
    if(!g_pApp)
        g_pApp = new TentApp();                                                

    // Construct path
    std::string szSavePath(g_ConfigDirectory);
    CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(g_szAppData);

    g_pApp->LoadFromFile(szSavePath);

    std::string buffer;
    JsonSerializer::SerializeObject(g_pApp, buffer);
    //std::cout<<" BUFFER : " << buffer << std::endl;

    return ret::A_OK;
}
#include <list>
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
    std::cout<< "FILEPATH : " << filepath << std::endl;
    unsigned int size = utils::CheckFileSize(filepath);
    if(!size)
        return ret::A_FAIL_OPEN;
    
    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

    if(!ifs.is_open())
        return ret::A_FAIL_OPEN;

    char* pData = new char[size+1];
    memset(pData, 0, (size));
    pData[size]='\0';

    ifs.read(pData, size);
    ifs.close();

    // Multipart post
    std::string url(szUrl);
    std::string response;
    
    std::string filename;
    utils::ExtractFileName(filepath, filename);

    // construct chunk filepaths
    std::string chunkName; 
    fi->GetChunkName(chunkName);
    std::string chunkPath = g_TempDirectory;
    chunkPath.append("/");
    chunkPath.append(chunkName);
    chunkPath.append("_");

    std::string path;
    char buf[256];
    std::list<std::string> paths;

    for(unsigned int i=0; i< fi->GetChunkCount(); i++)
    {
        memset(buf, '\0', 256);
        snprintf(buf, 256, "%u", i);

        path.clear();
        path += chunkPath + buf;

        paths.push_back(path);

    }

    ConnectionManager::GetInstance()->HttpMultipartPost( url, 
                                                         NULL,
                                                         postBuffer, 
                                                         &paths, 
                                                         response, 
                                                         g_at.GetMacAlgorithm(), 
                                                         g_at.GetAccessToken(), 
                                                         g_at.GetMacKey(), 
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

    if(pData)
    {
        delete[] pData;
        pData = 0;
    }

    return ret::A_OK;
}
// utility
//////
static int PutFile(const char* szUrl, const char* szFilePath, FileInfo* fi)
{
    // file path preferably to a chunked file.
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string postType("https://tent.io/types/post/attic/v0.1.0");

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
    std::cout<< "FILEPATH : " << filepath << std::endl;
    unsigned int size = utils::CheckFileSize(filepath);
    if(!size)
        return ret::A_FAIL_OPEN;
    
    std::cout<< " FILE SIZE : " << size << std::endl;
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
    
    std::string filename;
    utils::ExtractFileName(filepath, filename);

    // Set chunkfilepaths and pass that in

    // construct chunk filepaths
    std::string chunkName; 
    fi->GetChunkName(chunkName);
    std::string chunkPath = g_TempDirectory;
    chunkPath.append("/");
    chunkPath.append(chunkName);
    chunkPath.append("_");

    std::string path;
    char buf[256];
    std::list<std::string> paths;

    for(unsigned int i=0; i< fi->GetChunkCount(); i++)
    {
        memset(buf, '\0', 256);
        snprintf(buf, 256, "%u", i);

        path.clear();
        path += chunkPath + buf;

        paths.push_back(path);

    }


    ConnectionManager::GetInstance()->HttpMultipartPut( url, 
                                                        NULL,
                                                        postBuffer, 
                                                        &paths, 
                                                        response, 
                                                        g_at.GetMacAlgorithm(), 
                                                        g_at.GetAccessToken(), 
                                                        g_at.GetMacKey(), 
                                                        true);
 
    std::cout<<"RESPONSE : " << response << std::endl;

    JsonSerializer::DeserializeObject(&p, response);

    std::string postid = p.GetID();
    if(!postid.empty())
    {
       fi->SetPostID(postid); 
       fi->SetPostVersion(p.GetVersion()); // temporary for now, change later
       std::cout << " SIZE : " << p.GetAttachments()->size() << std::endl;
       std::cout << " Name : " << (*p.GetAttachments())[0]->Name << std::endl;
    }

    if(pData)
    {
        delete[] pData;
        pData = 0;
    }

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
    std::string postid;
    fi->GetPostID(postid);

    if(postid.empty())
    {
        // New Post
        // Construct post url
        std::string posturl = g_Entity;
        posturl += "/tent/posts";

        std::cout<< " POST URL : " << posturl << std::endl;

        return PostFile(posturl.c_str(), szFilePath, fi);
    }
    else
    {
        // Modify Post
        std::string posturl = g_Entity;
        posturl += "/tent/posts/";
        posturl += postid;

        std::cout<< " POST URL : " << posturl << std::endl;

        return PutFile(posturl.c_str(), szFilePath, fi);

    }

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
    CheckUrlAndAppendTrailingSlash(filepath);

    for(;itr != pEntryMap->end(); itr++)
    {
        std::string fn = itr->first;
        PullFile((filepath + fn).c_str());
    }
    
    return ret::A_OK;
}

int PullFile(const char* szFilePath)
{
    std::string filepath(szFilePath);
    std::string filename;

    utils::ExtractFileName(filepath, filename);

    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    // Search for file in manifest
    
    std::cout<<"FILE NAME : " << filename << std::endl;
    FileInfo* fi = g_pFileManager->GetFileInfo(filename);

    if(!fi)
    {
        std::cout<<"NULL FILE INFO"<<std::endl;
        
        return ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    }
    // Construct Post URL
    std::string postpath = g_Entity;
    postpath.append("/tent/posts/");
    std::string postid;
    fi->GetPostID(postid);
    postpath += postid;

    // Get Post
    std::string response;
    ConnectionManager::GetInstance()->HttpGetWithAuth ( postpath,
                                                        NULL,
                                                        response,
                                                        g_at.GetMacAlgorithm(), 
                                                        g_at.GetAccessToken(), 
                                                        g_at.GetMacKey(), 
                                                        true);


    std::cout << "RESPONSE : " << response << std::endl;

    // Deserialize response into post
    Post resp;

    JsonSerializer::DeserializeObject(&resp, response);

    std::cout << " Attachment Count : " << resp.GetAttachmentCount() << std::endl;
    // Construct list of attachments

    Post::AttachmentVec* av = resp.GetAttachments();
    Post::AttachmentVec::iterator itr = av->begin();

    std::string attachmentpath;
    std::string outpath;
   
    for(;itr != av->end(); itr++)
    {
        // Construct attachment path
        attachmentpath.clear();
        attachmentpath += postpath;
        attachmentpath.append("/attachments/");
        attachmentpath += (*itr)->Name;
        std::cout<< attachmentpath << std::endl;

        outpath.clear();
        outpath += g_TempDirectory;
        CheckUrlAndAppendTrailingSlash(outpath);
        outpath += (*itr)->Name;

        // Request attachment
        GetFileAndWriteOut(attachmentpath, outpath);
    }
    
    // Construct File
    std::cout << "STATUS : " << g_pFileManager->ConstructFile(filename) << std::endl;
    
    // Pull all attachments
    //
    // Reconstruct
    // If found get the post id and pull it

/*
    // Construct URL
    std::string attachmentpath = g_Entity;
    attachmentpath.append("/tent/posts/");
    attachmentpath += fi->GetPostID();
    attachmentpath.append("/attachments/");
    attachmentpath += filename;

    std::cout<<" ATTACHMENT PATH : " << attachmentpath << std::endl;

    std::string buf;
    //GetFile(attachmentpath, buf);
    GetFileAndWriteOut(attachmentpath, filepath);
*/
    /* 
    // write out to directory
    std::cout<<"FILE SIZE : " << buf.size() << std::endl;

    if(buf.size() > 0)
    {
        //std::string outpath = g_WorkingDirectory;
        std::string outpath = filepath; 
        //CheckUrlAndAppendTrailingSlash(outpath);
        //outpath += filename;
        std::ofstream ofs;
        std::cout<<outpath<<std::endl;

        ofs.open(outpath.c_str(), std::ofstream::out | std::ofstream::binary);

        if(ofs.is_open())
        {
            ofs.write(buf.c_str(), buf.size());
            ofs.close();
        }
        else
        {
            
        std::cout<< " HERE\n";
            return ret::A_FAIL_OPEN;
        }
    }
    */
    return ret::A_OK;
}

static int GetFileAndWriteOut(const std::string& url, const std::string &filepath)
{
    // file path preferably to a chunked file.
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    ConnectionManager::GetInstance()->HttpGetAttachmentWriteToFile( url, 
                                                                    NULL,
                                                                    filepath, 
                                                                    g_at.GetMacAlgorithm(), 
                                                                    g_at.GetAccessToken(), 
                                                                    g_at.GetMacKey(), 
                                                                    true);

    return ret::A_OK;
}

static int GetFile(const std::string& url, std::string &out)
{
    // file path preferably to a chunked file.
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string response;

    ConnectionManager::GetInstance()->HttpGetAttachment( url, 
                                                         NULL,
                                                         response, 
                                                         g_at.GetMacAlgorithm(), 
                                                         g_at.GetAccessToken(), 
                                                         g_at.GetMacKey(), 
                                                         true);

    std::cout << " RESPONSE : " << response << std::endl;

    Post p;
    JsonSerializer::DeserializeObject(&p, response);

    std::cout<< " Attachment size : " << p.GetAttachments()->size() << std::endl;

    std::string a;
    JsonSerializer::SerializeObject(&p, a);
    std::cout << " SERIALIZED : " << a << std::endl;

    out = response;
 
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
                        CheckUrlAndAppendTrailingSlash(path);
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

