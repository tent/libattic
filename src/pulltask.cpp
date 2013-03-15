#include "pulltask.h"

#include <iostream>
#include <boost/timer/timer.hpp>

#include "filesystem.h"
#include "filemanager.h"
#include "errorcodes.h"
#include "utils.h"
#include "compression.h"
#include "syncfiletask.h"
#include "eventsystem.h"
#include "netlib.h"
#include "taskdelegate.h"

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
                    TaskDelegate* callbackDelegate)
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
                              callbackDelegate)
{
}

PullTask::~PullTask()
{
}

void PullTask::RunTask() {
    std::string filepath;
    GetFilepath(filepath);

    event::RaiseEvent(event::Event::PULL, event::Event::START, filepath, NULL);
    int status = PullFile(filepath);
    event::RaiseEvent(event::Event::PULL, event::Event::DONE, filepath, NULL);

    Callback(status, filepath);
    SetFinishedState();
}

int PullTask::PullFile(const std::string& filepath) {
    int status = ret::A_OK;

    FileManager* fm = GetFileManager();
    FileInfo* fi = fm->GetFileInfo(filepath);                                        

    if(fi) {
        if(fi->GetDeleted())
            return ret::A_FAIL_PULL_DELETED_FILE;

        // Get relative filepath
        std::string relative_filepath;
        fi->GetFilepath(relative_filepath);

        Credentials fileCred;
        status = RetrieveFileCredentials(fi, fileCred);

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
                        jsn::DeserializeObject(&p, response.body);
                        status = RetrieveFile(relative_filepath, 
                                              chunkposturl, 
                                              fileCred, 
                                              p, 
                                              fi);
                        // File retrieval was successfull, post step
                        if(status == ret::A_OK) {
                            // Update version
                            char szVer[256] = {'\0'};
                            snprintf(szVer, 256, "%d", p.GetVersion());
                            fm->SetFileVersion(relative_filepath, std::string(szVer));
                        }

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

int PullTask::RetrieveFileCredentials(FileInfo* fi, Credentials& out) {
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
            if(jsn::DeserializeObject(&ap, resp.body)) {
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
                //std::cout<<" FILE KEY : " << filekey << std::endl;

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

int PullTask::RetrieveFile( const std::string filepath, 
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
    std::string fp, filename;
    fi->GetFilename(filename);
    fi->GetFilepath(fp);
    // This should be relative ie: <working>/some/path/file.txt
    std::cout<< " filepath : " << filepath << std::endl;
    std::cout<< " filepath from fi : " << fp << std::endl;
    std::cout<< " filename : " << filename << std::endl;

    std::string path;
    FileManager* fm = GetFileManager();

    fm->GetCanonicalFilepath(filepath, path);

    std::cout<< " path : " << path << std::endl;
    // check if we need to create folders
    fs::CreateDirectoryTree(path); // safer to pass canonical 

    std::string filekey;
    fCred.GetKey(filekey);
    std::cout<< " file key : " << filekey << std::endl;

    std::string temppath, randstr;
    GetTempDirectory(temppath);
    //crypto::GenerateRandomString(randstr, 4);
    utils::GenerateRandomString(randstr, 16);
    temppath += "/" + filename + "_" + randstr;

    std::cout<<" TEMPPATH : " << temppath << std::endl;

    std::ofstream ofs;
    ofs.open(temppath.c_str(),  std::ios::out | std::ios::trunc | std::ios::binary);
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
            status = RetrieveAttachment(attachmentpath, at, buffer);
            if(status == ret::A_OK) {
                // Transform Chunk
                ChunkInfo* ci = fi->GetChunkInfo((*itr).Name);
                std::string chunk;
                status = TransformChunk(ci, filekey, buffer, chunk);
                if(status == ret::A_OK) {
                    // Append to file 
                    ofs.write(chunk.c_str(), chunk.size());
                    count++;
                }
            }

            if(status) // fail
                break;
        }

        ofs.close();
        // Copy
        std::cout<<" moving file to : " << path << std::endl;
        if(status == ret::A_OK) {
            std::cout<<" file construction complete moving ... " << std::endl;
            fs::MoveFile(temppath, path);
        }
    }
    else {
        std::cout<<" FAIL TO OPEN FILE " << std::endl;
        status = ret::A_FAIL_OPEN_FILE;
    }
    
    return status;
}

int PullTask::TransformChunk(const ChunkInfo* ci, 
                             const std::string& filekey,
                             const std::string& chunkBuffer, 
                             std::string& out)
{
    int status = ret::A_OK;

    if(ci) {
        std::string iv;
        ci->GetIv(iv);

        Credentials cred;
        cred.SetKey(filekey);
        cred.SetIv(iv);

        std::cout<< " IV : " << iv << std::endl;
        std::cout<< " SIZEOF : " << chunkBuffer.size() << std::endl;

        // Base64Decode
        std::string base64Chunk;
        crypto::Base64DecodeString(chunkBuffer, base64Chunk);

        // Decrypt
        std::string decryptedChunk;
        status = crypto::DecryptStringCFB(base64Chunk, cred, decryptedChunk);
        //status = crypto::DecryptStringGCM(base64Chunk, fCred, decryptedChunk);

        if(status == ret::A_OK) {
            // Decompress
            std::string decompressedChunk;
            compress::DecompressString(decryptedChunk, decompressedChunk);
            //Verify chunk Check plaintext hmac
            std::string local_hash, plaintexthash;
            crypto::GenerateHash(decompressedChunk, local_hash);
            ci->GetPlainTextMac(plaintexthash);

            std::cout<<" LOCAL HASH : " << local_hash << std::endl;
            std::cout<<" CI HASH : " << plaintexthash << std::endl;

            if(local_hash == plaintexthash) {
                std::cout<<" ---- HASHES ARE THE SAME ---- " << std::endl;
                out = decompressedChunk;
                std::cout<<" decomp chunk : " << decompressedChunk.size() << std::endl;
                std::cout<<" size of chunk : " << out.size() << std::endl;
            }
            else
                status = ret::A_FAIL_INVALID_CHUNK_HASH;
        }
    }
    else
        status = ret::A_FAIL_INVALID_PTR;

    return status;
}

int PullTask::RetrieveAttachment( const std::string& url, 
                                  const AccessToken* at, 
                                  std::string& outBuffer)           
{                                                                                            
    int status = ret::A_OK;

    if(at) { 
        boost::timer::cpu_timer::cpu_timer t;
        Response response;
        status = netlib::HttpGetAttachment(url, NULL, at, response);
        boost::timer::cpu_times time = t.elapsed();
        long elapsed = time.user;
        // to milliseconds
        elapsed *= 0.000001;
        std::cout<<" ELAPSED : "<< elapsed << std::endl;
        if(elapsed > 0) {
            std::cout<<" buffer size : "<< outBuffer.size() << std::endl;
            unsigned int bps = (outBuffer.size()/elapsed);
            std::cout<<" BPS : " << bps << std::endl;
            // Raise event
            char szSpeed[256] = {'\0'};
            snprintf(szSpeed, 256, "%u", bps);
            event::RaiseEvent(event::Event::DOWNLOAD_SPEED, std::string(szSpeed), NULL);
        }

        if(response.code == 200) {
            outBuffer = response.body;
        }
        else {
            std::cout<<" RETREIVE ATTACHMENT FAILED " << std::endl;
            std::cout<<" FAILED BODY : " << response.body << std::endl;
            status = ret::A_FAIL_NON_200;
        }
    }
    else { 
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;                                                                        
}

int PullTask::GetChunkPost(FileInfo* fi, Response& responseOut) {
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
            status = netlib::HttpGetAttachment( posturl, NULL, at, responseOut);

            //std::cout<<" response out : " << responseOut.body << std::endl;
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

