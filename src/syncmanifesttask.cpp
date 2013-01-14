
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

    /*
    DeleteManifestPost(postid);
    return;
    */

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

        while(GetFileManager()->TryLock()) { sleep(0); }
        unsigned int localversion = GetFileManager()->GetManifestVersion();
        GetFileManager()->Unlock();
        unsigned int postversion = m_ManifestPost.GetVersion();

        std::cout<<"Post version : " << postversion << std::endl;

        // Compare version numbers
        if(localversion > postversion)
        {
            std::cout<<"higher"<<std::endl;
            // higher - upload
            //PushManifestPost(postid, &m_ManifestPost);
        }
        else if(localversion < postversion)
        {
            std::cout<<"lower"<<std::endl;
            // lower - download / overwrite / possibly merge

            // download manifest
            //PullManifestPostAttachment(postid);
                // write out to a temp file
                // do a comparision
                // unload old manifest
                // write into new manifest
                // reload manifest
        }
        else
        {
            std::cout<<"same"<<std::endl;

            PullManifestPostAttachment(postid);
            // same - merge
        }
    }
}

int SyncManifestTask::DeleteManifestPost(const std::string& postid)
{
    // Remove from filemanager
    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
    GetFileManager()->SetManifestPostID("");
    GetFileManager()->Unlock();

    // Remove from server
    // Construct Post URL                                
    std::string postpath;// = m_Entity;                  
    GetEntity(postpath);
    postpath.append("/tent/posts/");                     
    postpath += postid;                                  

    AccessToken* at = GetAccessToken();                                                              
    Response resp;
    ConnectionManager::GetInstance()->HttpDelete( postpath,
                                                  NULL,                            
                                                  resp,                        
                                                  at->GetMacAlgorithm(),           
                                                  at->GetAccessToken(),            
                                                  at->GetMacKey(),                 
                                                  true);                           

    std::cout<<" CODE : " << resp.code << std::endl;
    std::cout<<" COUT : " << resp.body << std::endl;

    return 0;
 

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

int SyncManifestTask::PullManifestPostAttachment(const std::string& postid)
{
    // Construct Post URL                                
    std::string postpath;// = m_Entity;                  
    GetEntity(postpath);
    postpath.append("/tent/posts/");                     
    postpath += postid;                                  

   // Assemble attachment path
    Post::AttachmentVec* av = m_ManifestPost.GetAttachments();             

    std::cout<< " VEC COUNT : " << av->size() << std::endl;      
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
        GetConfigDirectory(outpath);
                             
        utils::CheckUrlAndAppendTrailingSlash(outpath);          
        outpath += (*itr)->Name;                                 

        std::cout<<" NAME : " <<  (*itr)->Name << std::endl;     

        outpath += "_tmp";

        // Request attachment                                    
        GetFileAndWriteOut(attachmentpath, outpath);             
        // Decrypt
        std::string keypath;
        MasterKey mk;
        GetMasterKeyFromCredentials(mk, keypath);
       // Credentials cred = mk.GetCredentialsCopy();

        std::cout<<" Decrypting "<< std::endl;
        std::string path = outpath;
        path += "_hld";
        //m_Crypto.EncryptFile(outpath, path, cred);
 

    }                                                            

    return ret::A_OK;                                                                      
}

int SyncManifestTask::GetFileAndWriteOut(const std::string& url, const std::string &filepath)
{                                                                                                    
    //file path preferably to a chunked file.                                                       
    if(!GetTentApp())                                                                                
        return ret::A_FAIL_INVALID_APP_INSTANCE;                                                 

    Response response;
    AccessToken* at = GetAccessToken();                                                              
    ConnectionManager::GetInstance()->HttpGetAttachmentWriteToFile( url,                             
                                                                    NULL,                            
                                                                    response,
                                                                    filepath,                        
                                                                    at->GetMacAlgorithm(),           
                                                                    at->GetAccessToken(),            
                                                                    at->GetMacKey(),                 
                                                                    true);                           

    return ret::A_OK;                                                                                
}                                                                                                    

int SyncManifestTask::SearchForManifestPost(MetaStorePost& out)
{
    if(!GetTentApp())                                                                        
        return ret::A_FAIL_INVALID_APP_INSTANCE;                                         

    if(!GetFileManager())                                                                    
        return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;                                 

    // Prepare URL TODO :: operations like this should be abstracted out somewhere perhaps utils
    std::string url;                                                                         
    GetEntity(url);                                                                          
    url += "/tent/posts";                                                                    

    UrlParams params;                                                                        
    params.AddValue(std::string("post_types"), std::string(cnst::g_szAtticMetaStorePostType));              

    AccessToken* at = GetAccessToken();                                                      

    Response response;                                                                    
    ConnectionManager::GetInstance()->HttpGetWithAuth( url,                                  
                                                       &params,                              
                                                       response,                             
                                                       at->GetMacAlgorithm(),                
                                                       at->GetAccessToken(),                 
                                                       at->GetMacKey(),                      
                                                       true);                                

    std::cout<< " CODE : " << response.code <<std::endl;
    std::cout<< " RESPONSE : " << response.body << std::endl;                                     
    Json::Value root;                                                                            
    Json::Reader reader;                                                                         

    std::cout<<"HERE " << std::endl;
    if(!reader.parse(response.body, root))                                                            
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

int SyncManifestTask::PushManifestPost(const std::string& postid, MetaStorePost* post)
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
    if(postid.empty())
    {
        // New Post                                                         
        std::cout<< " POST URL : " << posturl << std::endl;                 

        unsigned int size = utils::CheckFilesize(filepath);                 
        MetaStorePost p;                                                        
        CreateManifestPost(p);

        std::string tempdir;                                                
        GetTempDirectory(tempdir);                                          

        filepath += "_enc";
        std::cout<<" FILEPATH : " << filepath << std::endl;
        PostManifest(posturl, filepath);
        /*
        AccessToken* at = GetAccessToken();                                 
        status = conops::PostFile( posturl,                                 
                                   filepath,                                
                                   tempdir,                                 
                                   GetFileManager(),                        
                                   GetConnectionManager(),                  
                                   fi,                          
                                   &p,                                      
                                   *at);                                    
         */                                                 
    }
    else
    {
        posturl += "/";
        posturl += postid;
        fi->SetPostID(postid);
        // MULTIPART PUT
        //
        //
        std::cout<< " PUT URL : " << posturl << std::endl;                 

        //unsigned int size = utils::CheckFilesize(filepath);                 
        MetaStorePost p;                                                        
        CreateManifestPost(p);

        std::string tempdir;                                                
        GetTempDirectory(tempdir);                                          
        
        filepath += "_enc";
        std::cout<<" FILEPATH : " << filepath << std::endl;        

        PutManifest(posturl, filepath);
        /*
        AccessToken* at = GetAccessToken();                                 
        status = conops::PutFile( posturl,                                 
                                  filepath,                                
                                  tempdir,                                 
                                  GetFileManager(),                        
                                  GetConnectionManager(),                  
                                  fi,                          
                                  &p,                                      
                                  *at);                                    
                                  */
    }

    return status;
}

int SyncManifestTask::PostManifest(const std::string& url, const std::string& filepath)
{
    std::list<std::string> paths;
    paths.push_back(filepath);

    // Multipart post

    std::string postBuffer;
    JsonSerializer::SerializeObject(&m_ManifestPost, postBuffer);

    Response response;
    AccessToken* at = GetAccessToken();
    ConnectionManager::GetInstance()->HttpMultipartPost( url, 
                                                         NULL,
                                                         postBuffer, 
                                                         &paths, 
                                                         response, 
                                                         at->GetMacAlgorithm(), 
                                                         at->GetAccessToken(), 
                                                         at->GetMacKey(), 
                                                         true);

    std::cout<<"CODE : " << response.code << std::endl;
    std::cout<<"RESPONSE : " << response.body << std::endl;

    return 0;
}

int SyncManifestTask::PutManifest(const std::string& url, const std::string& filepath)
{
    std::list<std::string> paths;
    paths.push_back(filepath);

    // Multipart post

    std::string postBuffer;
    JsonSerializer::SerializeObject(&m_ManifestPost, postBuffer);

    Response response;
    AccessToken* at = GetAccessToken();
    ConnectionManager::GetInstance()->HttpMultipartPut( url, 
                                                        NULL,
                                                        postBuffer, 
                                                        &paths, 
                                                        response, 
                                                        at->GetMacAlgorithm(), 
                                                        at->GetAccessToken(), 
                                                        at->GetMacKey(), 
                                                        true);

    std::cout<<"CODE : " << response.code << std::endl;
    std::cout<<"RESPONSE : " << response.body << std::endl;


    return 0;
}

FileInfo* SyncManifestTask::CreateManifestFileInfoAndEncrypt()
{
    /*
    std::string path;
    MasterKey mk;
    GetMasterKeyFromCredentials(mk, path);

    Credentials cred = mk.GetCredentialsCopy();

    std::cout<<" Encrypting "<< std::endl;
    std::string outPath = path;
    outPath += "_enc";

    m_Crypto.EncryptFile(path, outPath, cred);
 
    FileInfo* fi = NULL;

    {
        while(GetFileManager()->TryLock()) { sleep(0); }
        fi = GetFileManager()->CreateFileInfo( cnst::g_szManifestName,
                                               path,
                                               outPath,
                                               "1",                
                                               "",
                                               "",
                                               "",
                                               cred.m_Key,                       
                                               cred.m_Iv);                       

        //GetFileManager()->IndexFile( path,
        //                            false,
        //                           fi);

        GetFileManager()->Unlock();

    }


    return fi;

    */
    return NULL;
}

void SyncManifestTask::GetMasterKeyFromCredentials(MasterKey& mk, std::string& outpath)
{
    // Construct Path
    GetCredentialsManager()->GetManifestPath(outpath);
    // Get Master Key
    GetCredentialsManager()->GetMasterKeyCopy(mk);
}
