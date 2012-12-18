
#include "syncmanifesttask.h"

#include "urlparams.h"
#include "constants.h"
#include "conoperations.h"
#include "utils.h"

SyncManifestTask::SyncManifestTask( TentApp* pApp, 
                  FileManager* pFm, 
                  ConnectionManager* pCon, 
                  CredentialsManager* pCm,
                  const AccessToken& at,
                  const std::string& entity,
                  const std::string& filepath,
                  const std::string& tempdir, 
                  const std::string& workingdir,
                  const std::string& configdir,
                  void (*callback)(int, void*))
                  :
                  TentTask( pApp,
                            pFm,
                            pCon,
                            pCm,
                            at,
                            entity,
                            filepath,
                            tempdir,
                            workingdir,
                            configdir,
                            callback)
{

}

SyncManifestTask::~SyncManifestTask()
{

}

void SyncManifestTask::RunTask()
{
    std::cout<<"running sync manifest task"<<std::endl;
    std::string postid;
    // Get Metadata Post id
    GetManifestPostID(postid);
    std::cout<<"Post ID : " << postid << std::endl;

    if(postid.empty())
    {
        // Create new metadata post
        MetaStorePost post;
        CreateManifestPost(post);
        PushManifestPost(postid, &post);
    }
    else
    {
        // Pull Metadata Post
        SearchForManifestPost(m_ManifestPost);

        while(!GetFileManager()->TryLock()) { sleep(0); }
        unsigned int localversion = GetFileManager()->GetManifestVersion();
        GetFileManager()->Unlock();
        unsigned int postversion = m_ManifestPost.GetVersion();

        std::cout<<"Post version : " << postversion << std::endl;

        // Compare version numbers
        if(localversion > postversion)
        {
            std::cout<<"higher"<<std::endl;
            // higher - upload
        }
        else if(localversion < postversion)
        {
            std::cout<<"lower"<<std::endl;
            // lower - download / overwrite / possibly merge
        }
        else
        {
            std::cout<<"same"<<std::endl;
            // same - merge
        }
    }
}

void SyncManifestTask::GetManifestPostID(std::string& out)
{
    // Get Metadata Post id
    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
    GetFileManager()->GetManifestPostID(out);
    GetFileManager()->Unlock();

    if(out.empty())
    {
        // Search the server for metastore post
        SearchForManifestPost(m_ManifestPost);
        m_ManifestPost.GetID(out);
        std::cout<<" OUT POST ID : " << out << std::endl;

        if(!out.empty())
        {
            // If exists insert post id to manifest
            while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
            GetFileManager()->SetManifestPostID(out);
            GetFileManager()->Unlock();
        }
    }
}

void SyncManifestTask::PullManifestPost(const std::string id)
{

}

int SyncManifestTask::SearchForManifestPost(MetaStorePost& out)
{
    if(!GetTentApp())                                                                        
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;                                         

    if(!GetFileManager())                                                                    
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;                                 

    // Prepare URL TODO :: operations like this should be abstracted out somewhere perhaps utils
    std::string url;                                                                         
    GetEntity(url);                                                                          
    url += "/tent/posts";                                                                    

    UrlParams params;                                                                        
    params.AddValue(std::string("post_types"), std::string(cnst::g_szAtticMetaStorePostType));              

    AccessToken* at = GetAccessToken();                                                      

    std::string response;                                                                    
    ConnectionManager::GetInstance()->HttpGetWithAuth( url,                                  
                                                       &params,                              
                                                       response,                             
                                                       at->GetMacAlgorithm(),                
                                                       at->GetAccessToken(),                 
                                                       at->GetMacKey(),                      
                                                       true);                                

    std::cout<< " RESPONSE : " << response << std::endl;                                     
    Json::Value root;                                                                            
    Json::Reader reader;                                                                         

    std::cout<<"HERE " << std::endl;
    if(!reader.parse(response, root))                                                            
        return ret::A_FAIL_JSON_PARSE; // TODO :: Create failed to parse message

    std::cout<<"HERE " << std::endl;

    Json::ValueIterator itr = root.begin();                                                      

    int count = 0;                                                                               
    for(;itr != root.end(); itr++)                                                               
    {                                                                                            
        out.Deserialize(*itr);                                                                     

        count++;                                                                                 

        std::cout<< "POST TYPE : " << std::endl;                                                 
        std::string posttype;                                                                    
        out.GetPostType(posttype);                                                                 
        std::cout<<posttype<<std::endl;

        // if proper post type
        if(posttype.compare(cnst::g_szAtticMetaStorePostType) == 0)
        {
            std::cout<<" Proper Post type " << std::endl;
            break; // TODO :: if there are more than one metastore post types there is a problem
                   // just grab the first for now.
        }
    }

    std::cout<<" COUNT : " << count << std::endl;

    return ret::A_OK;
}

void SyncManifestTask::CreateManifestPost(MetaStorePost& post)
{
    std::string root;

    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
    GetFileManager()->GetWorkingDirectory(root);
    GetFileManager()->Unlock();

    post.SetPermission(std::string("public"), false);
    post.MetaSetAtticRoot(root);
}

int SyncManifestTask::PushManifestPost(const std::string& postID, MetaStorePost* post)
{
    if(!post)
        return ret::A_FAIL_INVALID_PTR;

    // Create ephemeral fileinfo object for manifest
    FileInfo* fi = CreateManifestFileInfoAndEncrypt();
    // determine where it's located // Its in the config folder, always
    std::string filepath, filename;
    fi->GetFilepath(filepath);
    fi->GetFilename(filename);
    // index to filemanger, do not insert to manifest
    //
    // Take meta data post,
    // attach manifest to it
    
    std::string posturl;               
    GetEntity(posturl);                
    posturl += "/tent/posts";          
    
    int status = ret::A_OK;
    // Create Post Path
    if(postID.empty())
    {
        // New Post                                                         
        std::cout<< " POST URL : " << posturl << std::endl;                 

        unsigned int size = utils::CheckFilesize(filepath);                 
        MetaStorePost p;                                                        
        CreateManifestPost(p);

        std::string tempdir;                                                
        GetTempDirectory(tempdir);                                          

        AccessToken* at = GetAccessToken();                                 
        status = conops::PostFile( posturl,                                 
                                   filepath,                                
                                   tempdir,                                 
                                   GetFileManager(),                        
                                   GetConnectionManager(),                  
                                   fi,                          
                                   &p,                                      
                                   *at);                                    
                                                          
    }
    else
    {
        posturl += "/";
        posturl += postID;
        fi->SetPostID(postID);
        // MULTIPART PUT
        //
        //
        std::cout<< " POST URL : " << posturl << std::endl;                 

        unsigned int size = utils::CheckFilesize(filepath);                 
        MetaStorePost p;                                                        
        CreateManifestPost(p);

        std::string tempdir;                                                
        GetTempDirectory(tempdir);                                          

        AccessToken* at = GetAccessToken();                                 
        status = conops::PutFile( posturl,                                 
                                  filepath,                                
                                  tempdir,                                 
                                  GetFileManager(),                        
                                  GetConnectionManager(),                  
                                  fi,                          
                                  &p,                                      
                                  *at);                                    
    }

    return status;
}

FileInfo* SyncManifestTask::CreateManifestFileInfoAndEncrypt()
{
    std::string path;
    MasterKey mk;

    {
        while(!GetCredentialsManager()->TryLock()) { sleep(0); }
 
        // Construct Path
        GetCredentialsManager()->ConstructManifestPath(path);
        // Get Master Key
        mk = GetCredentialsManager()->GetMasterKeyCopy();
    
        GetCredentialsManager()->Unlock();
    }

    Credentials cred = mk.GetCredentialsCopy();

    std::cout<<" Encrypting "<< std::endl;
    std::string outPath = path;
    outPath += "_enc";

    m_Crypto.EncryptFile(path, outPath, cred);
 
    FileInfo* fi = NULL;

    {
        while(!GetFileManager()->TryLock()) { sleep(0); }
        fi = GetFileManager()->CreateFileInfo( cnst::g_szManifest,
                                               path,
                                               outPath,
                                               "1",                
                                               "",
                                               "",
                                               "",
                                               cred.key,                       
                                               cred.iv);                       

        //GetFileManager()->IndexFile( path,
        //                            false,
        //                           fi);

        GetFileManager()->Unlock();

    }

    return fi;
}
