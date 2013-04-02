#include "getfilestrategy.h"

#include <iostream>
#include <boost/timer/timer.hpp>

#include "utils.h"
#include "netlib.h"
#include "compression.h"
#include "crypto.h"
#include "filesystem.h"
#include "postutils.h"
#include "event.h"
#include "logutils.h"

namespace attic { 

GetFileStrategy::GetFileStrategy() {}
GetFileStrategy::~GetFileStrategy() {}

int GetFileStrategy::Execute(FileManager* pFileManager,
                             CredentialsManager* pCredentialsManager,
                             Response& out) {
    int status = ret::A_OK;
    file_manager_ = pFileManager;
    credentials_manager_ = pCredentialsManager;
    if(!file_manager_) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    if(!credentials_manager_) return ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    credentials_manager_->GetAccessTokenCopy(access_token_);

    post_path_ = GetConfigValue("post_path");
    std::string filepath = GetConfigValue("filepath");
    std::string post_attachment = GetConfigValue("post_attachment");

    FileInfo* fi = file_manager_->GetFileInfo(filepath);                                        
    if(fi) {
        if(fi->deleted())
            return ret::A_FAIL_PULL_DELETED_FILE;

        Credentials fileCred;
        status = RetrieveFileCredentials(fi, fileCred);

        std::string chunkpostid = fi->chunk_post_id();
        if(status == ret::A_OK) {
            if(!chunkpostid.empty()) {
                // Construct Post URL
                std::string posturl;
                utils::FindAndReplace(post_path_, "{post}", chunkpostid, posturl);
                std::string attachurl;
                utils::FindAndReplace(post_attachment, "{post}", chunkpostid, attachurl);

                Response response;
                status = GetChunkPost(fi, response);

                if(status == ret::A_OK) {
                    if(response.code == 200) {
                        // Get relative filepath
                        std::string relative_filepath = fi->filepath();
                        std::cout<<" chunk post : " << response.body << std::endl;

                        Post p;
                        jsn::DeserializeObject(&p, response.body);

                        try{
                            status = RetrieveFile(relative_filepath, 
                                                  attachurl,
                                                  fileCred, 
                                                  p, 
                                                  fi);
                        }
                        catch(std::exception& e) {
                            std::string excp = e.what();
                            std::string err = " Exception get file strategy exception : " + excp + "\n";
                            
                            log::LogException("SFN#985412", err); 

                        }
                        // File retrieval was successfull, post step
                        if(status == ret::A_OK) {
                            // Update version
                            char szVer[256] = {'\0'};
                            std::cout<<" FIX THIS IN GETFILESTRATEGY " << std::endl;
                            //snprintf(szVer, 256, "%d", p.GetVersion());
                            //file_manager_->SetFileVersion(relative_filepath, std::string(szVer));
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

int GetFileStrategy::RetrieveFileCredentials(FileInfo* fi, Credentials& out) {
    int status = ret::A_OK;
    if(fi) {
        std::cout<<"RETRIEVING CREDENTIALS " << std::endl;
        std::string posturl;
        std::string postid = fi->post_id();
        utils::FindAndReplace(post_path_, "{post}", postid, posturl);

        // Get Metadata post
        Response resp;
        netlib::HttpGet(posturl, NULL, &access_token_, resp);

        std::cout<<" POST URL : "<< posturl << std::endl;
        std::cout<<" CODE : " << resp.code << std::endl;
        std::cout<<" BODY : " << resp.body << std::endl;

        if(resp.code == 200) {
            FilePost fp;
            if(jsn::DeserializeObject(&fp, resp.body)) {
                std::string key, iv;
                key = fp.key_data();
                iv = fp.iv_data();

                MasterKey mKey;
                credentials_manager_->GetMasterKeyCopy(mKey);

                std::string mk;
                mKey.GetMasterKey(mk);
                Credentials FileKeyCred;
                FileKeyCred.set_key(mk);
                FileKeyCred.set_iv(iv);

                // Decrypt File Key
                std::string filekey;
                crypto::DecryptStringCFB(key, FileKeyCred, filekey);
                std::cout<<" FILE KEY : " << filekey << std::endl;

                out.set_key(filekey);
                out.set_iv(iv);
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

int GetFileStrategy::GetChunkPost(FileInfo* fi, Response& responseOut) {
    int status = ret::A_OK;

    if(fi) {
        // Construct Post URL                                                                        

        std::string posturl;

        std::string postid = fi->chunk_post_id();
        utils::FindAndReplace(post_path_, "{post}", postid, posturl);

        std::cout<<" Post path : " << posturl << std::endl;
        status = netlib::HttpGetAttachment( posturl, NULL, &access_token_, responseOut);

        if(status == ret::A_OK) {
            if(responseOut.code != 200)
                status = ret::A_FAIL_NON_200;
        }
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int GetFileStrategy::RetrieveFile(const std::string& filepath, 
                                  const std::string& postpath, 
                                  const Credentials& fileCred,
                                  Post& post,
                                  FileInfo* fi) {
    int status = ret::A_OK;
    // Construct list of attachments                                                             
    Post::AttachmentVec* av = post.GetAttachments();                                             
    Post::AttachmentVec::iterator itr = av->begin();                                             
    std::string attachmentpath, outpath;
    
    Credentials fCred = fileCred;
    std::string fp = fi->filepath();
    std::string filename = fi->filename();

    // This should be relative ie: <working>/some/path/file.txt
    std::cout<< " filepath : " << filepath << std::endl;
    std::cout<< " filepath from fi : " << fp << std::endl;
    std::cout<< " filename : " << filename << std::endl;

    std::string path;
    file_manager_->GetCanonicalFilepath(filepath, path);

    std::cout<< " path : " << path << std::endl;
    // check if we need to create folders
    fs::CreateDirectoryTree(path); // safer to pass canonical 

    std::string filekey =  fCred.key();
    std::cout<< " file key : " << filekey << std::endl;

    std::string temppath, randstr;
    file_manager_->GetTempDirectory(temppath);
    //crypto::GenerateRandomString(randstr, 4);
    utils::GenerateRandomString(randstr, 16);
    temppath += "/" + filename + "_" + randstr;

    std::cout<<" TEMPPATH : " << temppath << std::endl;

    std::ofstream ofs;
    ofs.open(temppath.c_str(),  std::ios::out | std::ios::trunc | std::ios::binary);
    if (ofs.is_open()) {
        int count = 0;
        std::cout<<" ATTACHMENT COUNT : " << av->size() << std::endl;
        for(;itr != av->end(); itr++) {
            // Construct attachment path

            utils::FindAndReplace(postpath, "{version}", post.version_id(), attachmentpath);
            utils::FindAndReplace(attachmentpath, "{name}", (*itr).name, attachmentpath);
            std::cout<<" attachment path : " << attachmentpath << std::endl;

            char szCount[256]={'\0'};
            snprintf(szCount, 256, "%d", count);

            outpath.clear();
            file_manager_->GetTempDirectory(outpath);

            utils::CheckUrlAndAppendTrailingSlash(outpath);
            outpath += (*itr).name;

            // Request attachment                                                                
            std::string buffer;
            status = RetrieveAttachment(attachmentpath, buffer);

            if(buffer.size() <= 0) {
                status = ret::A_FAIL_ZERO_SIZE;
            }

            if(status == ret::A_OK) {
                // Transform Chunk
                ChunkInfo* ci = fi->GetChunkInfo((*itr).name);
                std::string chunk;
                status = TransformChunk(ci, filekey, buffer, chunk);
                if(status == ret::A_OK) {
                    // Append to file 
                    ofs.write(chunk.c_str(), chunk.size());
                    count++;
                }
                else {
                    std::cout<<" FAILED TO TRANSFORM CHUNK : " << status << std::endl;
                }
            }

            if(status) // fail
                break;
        }

        ofs.close();
        // Copy

        if(status == ret::A_OK) {
            std::cout<<" moving file to : " << path << std::endl;
            // TODO :: this can be moved up the chain a bit, perhaps perform some other checks
            // before moving
            std::cout<<" file construction complete moving ... " << std::endl;
            fs::MoveFile(temppath, path);
        }

        // Cleanup temppath
        fs::DeleteFile(temppath);

    }
    else {
        std::cout<<" FAIL TO OPEN FILE " << std::endl;
        status = ret::A_FAIL_OPEN_FILE;
    }
    
    return status;
}

int GetFileStrategy::RetrieveAttachment(const std::string& url, std::string& outBuffer) {
    int status = ret::A_OK;

    boost::timer::cpu_timer::cpu_timer t;
    Response response;
    status = netlib::HttpGetAttachment(url, NULL, &access_token_, response);
    boost::timer::cpu_times time = t.elapsed();

    if(response.code == 200) {
        outBuffer = response.body;
    }
    else {
        std::cout<<" RETREIVE ATTACHMENT FAILED " << std::endl;
        std::cout<<" FAILED BODY : " << response.body << std::endl;
        status = ret::A_FAIL_NON_200;
    }
    
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

    
    return status;                                                                        
}

int GetFileStrategy::TransformChunk(const ChunkInfo* ci, 
                                    const std::string& filekey,
                                    const std::string& chunkBuffer, 
                                    std::string& out)
{
    int status = ret::A_OK;

    if(ci) {
        std::string iv = ci->iv();

        Credentials cred;
        cred.set_key(filekey);
        cred.set_iv(iv);

        std::cout<< " key : " << filekey << std::endl;;
        std::cout<< " IV : " << iv << std::endl;
        std::cout<< " SIZEOF : " << chunkBuffer.size() << std::endl;

        std::string rawhash;
        crypto::GenerateHash(chunkBuffer, rawhash);
        std::cout<<" RAW HASH : " << rawhash << std::endl;

        // Base64Decode
        std::string base64Chunk;
        crypto::Base64DecodeString(chunkBuffer, base64Chunk);

        std::string cipherhash;
        crypto::GenerateHash(base64Chunk, cipherhash);
        std::cout<<" CIPHER HASH : " << cipherhash << std::endl;
        // Decrypt
        std::string decryptedChunk;
        status = crypto::DecryptStringCFB(base64Chunk, cred, decryptedChunk);
        //status = crypto::DecryptStringGCM(base64Chunk, fCred, decryptedChunk);

        if(status == ret::A_OK) {
            // Decompress
            std::string decryptedHash;
            crypto::GenerateHash(decryptedChunk, decryptedHash);
            std::cout<<" DECRYPTED HASH : " << decryptedHash << std::endl;
            std::cout<<" DECRYPTED SIZE : " << decryptedChunk.size() << std::endl;
            std::string decompressedChunk;
            status = compress::DecompressString(decryptedChunk, decompressedChunk);
            if(status == ret::A_OK) { 
                //Verify chunk Check plaintext hmac
                std::string local_hash;
                crypto::GenerateHash(decompressedChunk, local_hash);
                std::string plaintexthash = ci->plaintext_mac();

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
    }
    else
        status = ret::A_FAIL_INVALID_PTR;

    return status;
}

}//namespace
