
#include "pushtask.h"

#include <list>

#include "filemanager.h"
#include "connectionmanager.h"

#include "errorcodes.h"
#include "utils.h"

PushTask::PushTask( TentApp* pApp, 
                    FileManager* pFm, 
                    ConnectionManager* pCon, 
                    const AccessToken& at,
                    const std::string& entity,
                    const std::string& filepath,
                    const std::string& tempdir,
                    const std::string& workingdir,
                    const std::string& configdir,
                    void (*callback)(int, void*))
                    :
                    TentTask ( pApp,
                               pFm,
                               pCon,
                               at,
                               entity,
                               filepath,
                               tempdir,
                               workingdir,
                               configdir,
                               callback )
{

}

PushTask::~PushTask()
{

}

void PushTask::RunTask()
{
    // Run the task
    std::string filepath;
    GetFilepath(filepath);

    int status = PushFile(filepath);
    // Callback
    Callback(status, NULL);
}

int PushTask::PushFile(const std::string& filepath)
{
    if(!GetTentApp())
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!GetFileManager())
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    std::string fn;
    utils::ExtractFileName(filepath, fn);

    std::cout << " Getting file info ... " << std::endl;

    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0); } 
    FileInfo* fi = GetFileManager()->GetFileInfo(fn);
    GetFileManager()->Unlock();

    if(!fi)
    {
        int status = 0;

        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        std::cout << "INDEXING FILE : " << std::endl;
        status = GetFileManager()->IndexFile(filepath);
        GetFileManager()->Unlock();

        if(status != ret::A_OK)
            return status;

        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        fi = GetFileManager()->GetFileInfo(fn);
        GetFileManager()->Unlock();
    }

    // Check for existing post
    std::string postid;
    fi->GetPostID(postid);

    if(postid.empty())
    {
        // New Post
        // Construct post url
        std::string posturl;
        GetEntity(posturl);

        posturl += "/tent/posts";

        std::cout<< " POST URL : " << posturl << std::endl;

        return PostFile(posturl, filepath, fi);
    }
    else
    {
        // Modify Post
        std::string posturl;
        GetEntity(posturl);
        posturl += "/tent/posts/";
        posturl += postid;

        std::cout<< " PUT URL : " << posturl << std::endl;

        return PutFile(posturl, filepath, fi);

    }

    return ret::A_OK;
}

int PushTask::PostFile(const std::string& url, const std::string &filepath, FileInfo* fi)
{
    // file path preferably to a chunked file.
    if(!GetTentApp())
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
    std::string response;
    std::string filename;
    utils::ExtractFileName(filepath, filename);

    // construct chunk filepaths
    std::string chunkName; 
    fi->GetChunkName(chunkName);

    std::string chunkPath;// = m_TempDirectory;
    GetTempDirectory(chunkPath);
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
    
    std::cout<<"RESPONSE : " << response << std::endl;

    JsonSerializer::DeserializeObject(&p, response);

    int status = ret::A_OK;
    std::string postid = p.GetID();
    if(!postid.empty())
    {
        fi->SetPostID(postid); 
        fi->SetPostVersion(0); // temporary for now, change later
        std::cout << " SIZE : " << p.GetAttachments()->size() << std::endl;
        std::cout << " Name : " << (*p.GetAttachments())[0]->Name << std::endl;

        if(GetFileManager())
        {

            while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
            GetFileManager()->SetFilePostId(filename, postid);
            GetFileManager()->Unlock();
        }
        else
            std::cout<< "INVALID FILEMANAGER " << std::endl;

    }

    if(pData)
    {
        delete[] pData;
        pData = 0;
    }

    return status;
}

int PushTask::PutFile(const std::string& url, const std::string &filepath, FileInfo* fi)
{
    // file path preferably to a chunked file.
    if(!GetTentApp())
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
    std::string response;
    std::string filename;

    utils::ExtractFileName(filepath, filename);

    // Set chunkfilepaths and pass that in

    // construct chunk filepaths
    std::string chunkName; 
    fi->GetChunkName(chunkName);

    std::string chunkPath; // = m_TempDirectory;
    GetTempDirectory(chunkPath);
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
