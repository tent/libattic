#include "pulltask.h"

#include <iostream>

#include "filemanager.h"
#include "connectionmanager.h"

#include "errorcodes.h"
#include "utils.h"
#include "conoperations.h"
#include "compression.h"


PullTask::PullTask( TentApp* pApp, 
                    FileManager* pFm, 
                    CredentialsManager* pCm,
                    TaskArbiter* pTa,
                    TaskFactory* pTf,
                    const AccessToken& at,
                    const Entity& entity,
                    const std::string& filepath,
                    const std::string& tempdir,
                    const std::string& workingdir,
                    const std::string& configdir,
                    void (*callback)(int, void*))
                    :
                    TentTask( Task::PULL,
                              pApp,
                              pFm,
                              pCm,
                              pTa,
                              pTf,
                              at,
                              entity,
                              filepath,
                              tempdir,
                              workingdir,
                              configdir,
                              callback )
{

}

PullTask::~PullTask()
{

}

void PullTask::RunTask()
{
    std::string filepath;
    GetFilepath(filepath);
    //int status = PullFile(filepath);
    int status = PullFileNew(filepath);

    Callback(status, NULL);
    SetFinishedState();
}

int PullTask::PullFileNew(const std::string& filepath)
{
    int status = ret::A_OK;

    FileInfo* fi = GetFileManager()->GetFileInfo(filepath);                                        

    if(fi) {                                                                                            
        std::string postid;                                                                          
        fi->GetChunkPostID(postid);
        if(!postid.empty()) {
            // Construct Post URL                                                                        
            std::string posturl;
            ConstructPostUrl(posturl);
            utils::CheckUrlAndAppendTrailingSlash(posturl);
            posturl += postid;

            Response response;                                                                        
            status = GetChunkPost(fi, response);

            if(status == ret::A_OK) {
                if(response.code == 200) {
                    Post p;
                    JsonSerializer::DeserializeObject(&p, response.body);

                    status = RetreiveFile(filepath, posturl, p, fi);
                }
                else {
                    status = ret::A_FAIL_NON_200;
                }
            }
        }
        else {
            status = ret::A_FAIL_INVALID_POST_ID;
        }
    }
    else {
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;                                                 
    }

    return status;
}

int PullTask::RetreiveFile( const std::string filepath, 
                            const std::string postpath, 
                            Post& post,
                            FileInfo* fi)
{
    int status = ret::A_OK;

    // Construct list of attachments                                                             
    Post::AttachmentVec* av = post.GetAttachments();                                             
    Post::AttachmentVec::iterator itr = av->begin();                                             
    std::string attachmentpath, outpath;

    AccessToken* at = GetAccessToken();
    // Encrypt File Key
    MasterKey mKey;
    GetCredentialsManager()->GetMasterKeyCopy(mKey);

    std::string mk;
    mKey.GetMasterKey(mk);

    Credentials fCred;
    fCred.SetKey(mk);

    std::cout<< " filepath : " << filepath << std::endl;

    std::ofstream ofs;
    ofs.open(filepath.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);


    std::cout<<" FAILBIT : " << ofs.fail() << std::endl;
    std::cout<<" TRYING TO OPEN THE FILE " << std::endl;
    if (ofs.is_open()) {
        int count = 0;
        for(;itr != av->end(); itr++) {
            // Construct attachment path
            attachmentpath.clear();
            attachmentpath += postpath;
            attachmentpath.append("/attachments/");

            char szCount[256]={'\0'};
            snprintf(szCount, 256, "%d", count);

            attachmentpath += (*itr).Name;

            outpath.clear();
            GetTempDirectory(outpath);

            utils::CheckUrlAndAppendTrailingSlash(outpath);
            outpath += (*itr).Name;

            // Request attachment                                                                
            std::string buffer;
            RetrieveAttachment(attachmentpath, at, buffer);

            ChunkInfo* ci = fi->GetChunkInfo((*itr).Name);

            if(ci) {
                std::string iv;
                ci->GetIv(iv);
                fCred.SetIv(iv);

                std::cout<< " BUFFER SIZE : " << buffer.size() << std::endl;

                // Decrypt
                std::string decryptedChunk;
                status = crypto::DecryptStringCFB(buffer, fCred, decryptedChunk);

                if(status == ret::A_OK) {
                    std::cout<<" DECRYPTED STRING SIZE : " << decryptedChunk.size() << std::endl;

                    // Decompress
                    std::string decompressedChunk;
                    compress::DecompressString(decryptedChunk, decompressedChunk);

                    std::cout<< " DECOMPRESSED CHUNK SIZE : " << decompressedChunk.size() << std::endl;
                    // Write out
                    ofs.write(decompressedChunk.c_str(), decompressedChunk.size());
                }
                else {
                    std::cout << "FAIL TO DECRYPT : " << status << std::endl;
                }
            }

            else {
                // Abort
                status = ret::A_FAIL_INVALID_PTR;
                break;
            }

            count++;
        }

        ofs.close();
    }
    else {
        std::cout<<" FAIL TO OPEN FILE " << std::endl;
        status = ret::A_FAIL_OPEN_FILE;
    }
    
    return status;
}

int PullTask::RetrieveAttachment(const std::string& url, const AccessToken* at, std::string& outBuffer)           
{                                                                                            
    int status = ret::A_OK;

    if(at) { 
        Response response;
        status = netlib::HttpAsioGetAttachment(url, NULL, at, response);
        outBuffer = response.body;
    }
    else { 
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;                                                                        
}

int PullTask::PullFile(const std::string& filepath)
{                                                                                                
    //std::string filename;                                                                        
    //utils::ExtractFileName(filepath, filename);                                                  

    if(!GetFileManager())                                                                          
        return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;                                     

    std::cout<<" QUERY FILEPATH : " << filepath << std::endl;
    FileInfo* fi = GetFileManager()->GetFileInfo(filepath);                                        

    if(!fi) {                                                                                            
        return ret::A_FAIL_FILE_NOT_IN_MANIFEST;                                                 
    }                                                                                            

    // Construct Post URL                                                                        
    std::string posturl;
    ConstructPostUrl(posturl);

    std::string postid;                                                                          
    fi->GetChunkPostID(postid);
    posturl += "/" + postid;                                                                          

    int status = ret::A_OK;
    Response response;                                                                        
    status = GetChunkPost(fi, response);

    if(status == ret::A_OK) {
        /*
        std::cout<<" Chunk post response : " << response.code << std::endl;
        std::cout<<" Chunk post body : " << response.body << std::endl;
        */

        if(response.code == 200) {
            // Deserialize response into post
            Post resp;
            JsonSerializer::DeserializeObject(&resp, response.body);
            GetAttachmentsFromPost(posturl, resp);

            // Construct File                                                                        
            GetFileManager()->Lock();
            GetFileManager()->ConstructFileNew(filepath);
            GetFileManager()->Unlock();
        }
        else
        {
            status = ret::A_FAIL_NON_200;
        }
    }

    return status;
}

int PullTask::GetChunkPost(FileInfo* fi, Response& responseOut)
{
    int status = ret::A_OK;

    if(fi) {
        // Construct Post URL                                                                        
        std::string posturl;
        ConstructPostUrl(posturl);

        std::string postid;                                                            
        fi->GetChunkPostID(postid);
        posturl += "/" + postid;                                                                          

        std::cout<<" Post path : " << posturl << std::endl;
        // Get Post                                                                                  
        AccessToken* at = GetAccessToken();
        if(at) {
            status = netlib::HttpAsioGetAttachment( posturl, NULL, at, responseOut);

            std::cout<<" response out : " << responseOut.body << std::endl;
            if(status == ret::A_OK) {
                if(responseOut.code != 200)
                    status = ret::A_FAIL_NON_200;
            }
        }
        else {
            status = ret::A_FAIL_INVALID_PTR;
        }
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int PullTask::GetAttachmentsFromPost(const std::string postpath, Post& post)
{
    int status = ret::A_OK;

    // Construct list of attachments                                                             
    Post::AttachmentVec* av = post.GetAttachments();                                             
    Post::AttachmentVec::iterator itr = av->begin();                                             
    std::string attachmentpath, outpath;

    int count = 0;

    for(;itr != av->end(); itr++) {
        // Construct attachment path
        attachmentpath.clear();
        attachmentpath += postpath;
        attachmentpath.append("/attachments/");

        char szCount[256]={'\0'};
        snprintf(szCount, 256, "%d", count);

        attachmentpath += (*itr).Name;

        outpath.clear();
        GetTempDirectory(outpath);

        utils::CheckUrlAndAppendTrailingSlash(outpath);
        outpath += (*itr).Name;

        // Request attachment                                                                
        GetFileAndWriteOut(attachmentpath, outpath);
        count++;
    }                                                                                        
    
    return status;
}

int PullTask::GetFileAndWriteOut(const std::string& url, const std::string &filepath)           
{                                                                                            
    int status = ret::A_OK;

    Response response;
    AccessToken* at = GetAccessToken();

    if(at)
    {
        //status = conops::HttpGetAttachmentAndWriteOut(url, NULL, *at, filepath, response);
        status = conops::HttpGetAttachmentAndWriteOut( url, 
                                                       NULL, 
                                                       *at, 
                                                       filepath, 
                                                       GetConnectionHandle(), 
                                                       response);
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;                                                                        
}                                                                                            

int PullTask::GetDownloadSpeed()
{
    int speed = -1;
    if(GetConnectionHandle())
        speed = GetConnectionHandle()->GetDownloadSpeed();
    return speed;
}


