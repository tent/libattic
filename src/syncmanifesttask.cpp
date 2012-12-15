
#include "syncmanifesttask.h"

#include "urlparams.h"
#include "constants.h"

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
    std::string postid;
    // Get Metadata Post id
    GetManifestPostID(postid);
    if(postid.empty())
    {
        // Create new metadata post
        MetaStorePost post;
        CreateManifestPost(post);
        PushManifestPost(postid, &post);
    }

    // Pull Metadata Post
        // Compare versions
        // If server version newer, replace client version
            // This is more involved, if manifest is direct there needs to be some sort of merge
        // If client version is newer, PUT new post, (bump version number) 


}


void SyncManifestTask::GetManifestPostID(std::string&out)
{
    // Get Metadata Post id
    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
    GetFileManager()->GetManifestPostID(out);
    GetFileManager()->Unlock();

    if(out.empty())
    {
        // Search the server for metastore post
        MetaStorePost p;
        SearchForManifestPost(p);
        p.GetID(out);

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

    if(!reader.parse(response, root))                                                            
        return ret::A_FAIL_JSON_PARSE; // TODO :: Create failed to parse message

    Json::ValueIterator itr = root.begin();                                                      

    int count = 0;                                                                               
    for(;itr != root.end(); itr++)                                                               
    {                                                                                            
        JsonSerializer::DeserializeObject(&out, (*itr).asString());                              

        out.Deserialize(*itr);                                                                     
        count++;                                                                                 

        std::cout<< "POST TYPE : " << std::endl;                                                 
        std::string posttype;                                                                    
        out.GetPostType(posttype);                                                                 
        std::cout<<posttype<<std::endl;

        // if proper post type
        if(posttype.compare(cnst::g_szAtticMetaStorePostType) == 0)
        {
            std::cout<<" Proper Post type: " << std::endl;
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

void SyncManifestTask::PushManifestPost(const std::string& postID, MetaStorePost* post)
{
    if(!post)
        return;

    // Create ephemeral fileinfo object for manifest
    // determine where it's located
    // index to filemanger, do not insert to manifest
    // Take meta data post,
    // attach manifest to it
    
    // Create Post Path
    if(postID.empty())
    {
        // MULTIPART POST

    }
    else
    {
        // MULTIPART PUT
    }

}


