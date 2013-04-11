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
                             CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);

    post_path_ = GetConfigValue("post_path");
    std::string filepath = GetConfigValue("filepath");
    std::string post_attachment = GetConfigValue("post_attachment");

    FileInfo* fi = file_manager_->GetFileInfo(filepath);                                        
    if(fi) {
        if(fi->deleted())
            return ret::A_FAIL_PULL_DELETED_FILE;
        // Check for conflict
        PostTree file_tree;
        ConstructPostTree(fi, file_tree);

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

                        ChunkPost p;
                        jsn::DeserializeObject(&p, response.body);

                        try{
                            status = AssembleFile(relative_filepath,
                                                  attachurl,
                                                  fileCred,
                                                  p,
                                                  fi);
                        }
                        catch(std::exception& e) {
                            log::LogException("SFN#985412", e); 
                        }

                        if(status == ret::A_OK) {
                        }
                    }
                    else {
                        status = ret::A_FAIL_NON_200;
                        log::LogHttpResponse("14MGFS80", response);
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
                //crypto::DecryptStringCFB(key, FileKeyCred, filekey);
                status = crypto::DecryptStringGCM(key, FileKeyCred, filekey);
                std::cout<<" FILE KEY : " << filekey << std::endl;
                if(status == ret::A_OK) {
                    out.set_key(filekey);
                    out.set_iv(iv);
                }
                else { 
                    std::cout<<" FAILED TO BUILD FILE KEY " << std::endl;
                }
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

int GetFileStrategy::AssembleFile(const std::string& filepath,
                                  const std::string& url,
                                  const Credentials& file_cred,
                                  ChunkPost& post,
                                  FileInfo* fi) {
    int status = ret::A_OK;

    // Create temp path
    std::string fp = fi->filepath(); // filepath of file within attic dir
    std::string filename = fi->filename();

    std::string temp_path;
    file_manager_->GetTempDirectory(temp_path);
    utils::CheckUrlAndAppendTrailingSlash(temp_path);
    std::string randstr;
    utils::GenerateRandomString(randstr, 16);
    temp_path += filename + "_" + randstr;

    status = RetrieveAttachments(temp_path, url, file_cred, post, fi);
    if(status == ret::A_OK) {
        std::string path;
        file_manager_->GetCanonicalFilepath(filepath, path);
        fs::MoveFile(temp_path, path);
    }

    // delete temp file 
    fs::DeleteFile(temp_path);
    return status;
}
 
int GetFileStrategy::RetrieveAttachments(const std::string& filepath,
                                         const std::string& attachment_url,
                                         const Credentials& cred,
                                         ChunkPost& post,
                                         FileInfo* fi) {
    int status = ret::A_OK;
    if(post.chunk_info_list()->size()) {
        Post::AttachmentMap* av = post.attachments();                                             

        std::ofstream ofs;
        ofs.open(filepath.c_str(),  std::ios::out | std::ios::trunc | std::ios::binary);
        
        if (ofs.is_open()) {
            unsigned int count = post.chunk_info_list_size();
            for(unsigned int i=0; i < count; i++) {
                std::cout<<" FINDING CHUNK # : " << i << std::endl;
                ChunkPost::ChunkInfoList::iterator itr = post.chunk_info_list()->find(i);
                if(itr != post.chunk_info_list()->end()) {
                    std::cout<<" FOUND CHUNK # : " << i << std::endl;
                    // Get attachment
                    if(post.has_attachment(itr->second.chunk_name())) {
                        Attachment attch = post.get_attachment(itr->second.chunk_name());
                        std::string attachment_path;
                        utils::FindAndReplace(attachment_url,
                                              "{digest}",
                                              attch.digest,
                                              attachment_path);
                        /*
                        utils::FindAndReplace(attachment_url, 
                                              "{version}", 
                                              post.version_id(), 
                                              attachment_path);
                        utils::FindAndReplace(attachment_path, 
                                              "{name}", 
                                              itr->second.chunk_name(), 
                                              attachment_path);
                                              */
                        std::cout<<" attachment path : " << attachment_path << std::endl;
                        // Request attachment
                        std::string buffer;
                        status = RetrieveAttachment(attachment_path, buffer);
                        if(status == ret::A_OK) {
                            ChunkInfo* ci = fi->GetChunkInfo(itr->second.chunk_name());
                            std::string chunk;
                            status = TransformChunk(ci, cred.key(), buffer, chunk);
                            if(status == ret::A_OK) {
                                // Append to file 
                                ofs.write(chunk.c_str(), chunk.size());
                            }
                            else {
                                std::cout<<" FAILED TRANSFORM " << std::endl;
                                std::cout<<" STATUS : "<< status << std::endl;

                                break;
                            }
                        }
                    }
                }
                else {
                    std::cout<<" DID NOT FIND CHUNK # : " << i << std::endl;
                    std::cout<<" Out of sequence chunk " << std::endl;
                }
            }
            ofs.close();
        }
    }
    else {
        status = ret::A_FAIL_EMPTY_CHUNK_POST;
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
                                    std::string& out) {
    int status = ret::A_OK;

    if(ci) {
        std::string iv = ci->iv();

        Credentials cred;
        cred.set_key(filekey);
        cred.set_iv(iv);

        std::cout<< " TRANSFORMING CHUNK " << std::endl;
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
        std::string ci_cipherhash = ci->ciphertext_mac();
        if(ci_cipherhash == cipherhash) {
            std::cout<<" Ciphertext Hashes match! " << std::endl;
            std::cout<<" decrypting ... " << std::endl;
            // Decrypt
            std::string decryptedChunk;
            //status = crypto::DecryptStringCFB(base64Chunk, cred, decryptedChunk);
            status = crypto::DecryptStringGCM(base64Chunk, cred, decryptedChunk);
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
        else {
            std::string message("Invalid Ciphertext hash compare\n");
            message += " chunk hash : " + ci_cipherhash + "\n";
            message += " local hash : " + cipherhash + "\n";
            log::LogString("1PRF123X", message);
            status = ret::A_FAIL_INVALID_CHUNK_HASH;
        }
    }
    else
        status = ret::A_FAIL_INVALID_PTR;

    return status;
}

int GetFileStrategy::ConstructPostTree(FileInfo* fi, PostTree& tree) { 
    int status = ret::A_OK;
    if(fi) {
        std::cout<<"BUILDING POST TREE " << std::endl;
        std::string posturl;
        std::string postid = fi->post_id();

        status = RetrieveAndInsert(postid, tree);
    }
    return status;
}

int GetFileStrategy::RetrieveAndInsert(const std::string& postid, PostTree& tree) {
    int status = ret::A_OK;
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", postid, posturl);

    Response resp;
    netlib::HttpGet(posturl, NULL, &access_token_, resp);

    std::cout<<" POST URL : "<< posturl << std::endl;
    std::cout<<" CODE : " << resp.code << std::endl;
    std::cout<<" BODY : " << resp.body << std::endl;

    if(resp.code == 200) {
        FilePost fp;
        if(jsn::DeserializeObject(&fp, resp.body)) {
            tree.PushBackPost(&fp);
        }
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

}//namespace
