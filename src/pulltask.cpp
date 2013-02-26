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

int PullTask::RetreiveFileCredentials(FileInfo* fi, Credentials& out)
{
    int status = ret::A_OK;
    if(fi) {
        std::string posturl;
        ConstructPostUrl(posturl);

        std::string postid;
        fi->GetPostID(postid);
        utils::CheckUrlAndAppendTrailingSlash(posturl);
        posturl += postid;

        // Get Metadata post
        AccessToken* at = GetAccessToken();
        Response resp;
        netlib::HttpGet(posturl, NULL, at, resp);

        if(resp.code == 200) {
            AtticPost ap;
            if(JsonSerializer::DeserializeObject(&ap, resp.body)) {
                std::string key, iv;
                ap.GetAtticPostKeyData(key);
                ap.GetAtticPostIvData(iv);

                MasterKey mKey;
                GetCredentialsManager()->GetMasterKeyCopy(mKey);

                std::string mk;
                mKey.GetMasterKey(mk);
                Credentials FileKeyCred;
                FileKeyCred.SetKey(mk);
                FileKeyCred.SetIv(iv);

                // Decrypt File Key
                std::string filekey;
                crypto::DecryptStringCFB(key, FileKeyCred, filekey);

                std::cout<<" FILE KEY : " << filekey << std::endl;

                out.SetKey(filekey);
                out.SetIv(iv);
            }
        }
        else {
            status = ret::A_FAIL_NON_200;
        }
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int PullTask::PullFileNew(const std::string& filepath)
{
    int status = ret::A_OK;

    FileInfo* fi = GetFileManager()->GetFileInfo(filepath);                                        

    if(fi) {
        Credentials fileCred;
        status = RetreiveFileCredentials(fi, fileCred);

        std::string chunkpostid;
        fi->GetChunkPostID(chunkpostid);
        if(status == ret::A_OK) {
            if(!chunkpostid.empty()) {
                // Construct Post URL
                std::string posturl;
                ConstructPostUrl(posturl);

                std::string chunkposturl = posturl;
                utils::CheckUrlAndAppendTrailingSlash(chunkposturl);
                chunkposturl += chunkpostid;

                Response response;
                status = GetChunkPost(fi, response);

                if(status == ret::A_OK) {
                    if(response.code == 200) {
                        Post p;
                        JsonSerializer::DeserializeObject(&p, response.body);
                        status = RetreiveFile( filepath, 
                                               chunkposturl, 
                                               fileCred, 
                                               p, 
                                               fi);
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
            status = ret::A_FAIL_NO_CREDENTIALS;
        }
    }
    else {
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;                                                 
    }

    return status;
}

int PullTask::RetreiveFile( const std::string filepath, 
                            const std::string postpath, 
                            const Credentials& fileCred,
                            Post& post,
                            FileInfo* fi)
{
    int status = ret::A_OK;

    // Construct list of attachments                                                             
    Post::AttachmentVec* av = post.GetAttachments();                                             
    Post::AttachmentVec::iterator itr = av->begin();                                             
    std::string attachmentpath, outpath;

    AccessToken* at = GetAccessToken();
    
    Credentials fCred = fileCred;
    std::cout<< " filepath : " << filepath << std::endl;
    std::string fileKey;
    fCred.GetKey(fileKey);
    std::cout<< " file key : " << fileKey << std::endl;

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

                std::cout<< " IV : " << iv << std::endl;

                // Base64Decode
                std::string base64Chunk;
                crypto::Base64DecodeString(buffer, base64Chunk);

                // Decrypt
                std::string decryptedChunk;
                status = crypto::DecryptStringCFB(base64Chunk, fCred, decryptedChunk);
                //status = crypto::DecryptStringGCM(base64Chunk, fCred, decryptedChunk);

                if(status == ret::A_OK) {
                    // Decompress
                    std::string decompressedChunk;
                    compress::DecompressString(decryptedChunk, decompressedChunk);
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

int PullTask::RetrieveAttachment( const std::string& url, 
                                  const AccessToken* at, std::string& outBuffer)           
{                                                                                            
    int status = ret::A_OK;

    if(at) { 
        std::cout<<" retrieving attachment .... : " << url << std::endl;
        Response response;
        status = netlib::HttpAsioGetAttachment(url, NULL, at, response);

        if(response.code == 200) {
            outBuffer = response.body;
        }
        else {
            status = ret::A_FAIL_NON_200;
        }
    }
    else { 
        status = ret::A_FAIL_INVALID_PTR;
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

int PullTask::GetDownloadSpeed()
{
    int speed = -1;
    if(GetConnectionHandle())
        speed = GetConnectionHandle()->GetDownloadSpeed();
    return speed;
}


