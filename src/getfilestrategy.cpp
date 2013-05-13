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
#include "connectionhandler.h"
#include "pagepost.h"

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
    std::string entity = GetConfigValue("entity");

    if(!ValidMasterKey()) return ret::A_FAIL_INVALID_MASTERKEY;

    if(!filepath.empty()) { 
        FileInfo* fi = file_manager_->GetFileInfo(filepath);                                        
        if(fi) {
            std::cout<<" FILE : " << filepath << " DELETED : " << fi->deleted() << std::endl;
            if(fi->deleted()) return ret::A_FAIL_PULL_DELETED_FILE;
            // Retrieve file metadata
            FilePost meta_post;
            status = RetrieveFilePost(fi, meta_post);
            if(status == ret::A_OK) {
                Credentials file_cred;
                // Get file credentials
                status = ExtractCredentials(meta_post, file_cred);
                if(status == ret::A_OK) {
                    if(!file_cred.key_empty()) {
                        // Query for chunkposts mentioning metadata
                        ChunkPostList cp_list;
                        RetrieveChunkPosts(entity, meta_post.id(), cp_list);
                        // put file posts in order
                        // pull chunks
                        status = ConstructFile(cp_list, file_cred, fi);
                    }
                    else {
                        std::string error = "Invalid file key on get file";
                        log::LogString("MAOP8091", error);
                        status = ret::A_FAIL_INVALID_FILE_KEY;
                    }
                }
            }
        }
        else {
            status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
        }
    }
    else {
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
        std::cout<<" FILEPATH EMPTY : " << filepath << std::endl;
    }
    
    return status;
}
 
int GetFileStrategy::RetrieveFilePost(FileInfo* fi, FilePost& out) { 
    int status = ret::A_OK;
    std::cout<<" RETRIEVE FILE POST " << std::endl;;
    std::string posturl;
    std::string postid = fi->post_id();
    std::cout<<" POST ID : " << postid << std::endl;
    utils::FindAndReplace(post_path_, "{post}", postid, posturl);

    // Get Metadata post
    Response resp;
    netlib::HttpGet(posturl, NULL, &access_token_, resp);

    std::cout<<" POST URL : "<< posturl << std::endl;
    std::cout<<" CODE : " << resp.code << std::endl;
    std::cout<<" BODY : " << resp.body << std::endl;

    if(resp.code == 200) {
        if(!jsn::DeserializeObject(&out, resp.body)) {
            std::cout<<" FAILED TO DESERIALIZE FILE POST " << std::endl;
        }
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int GetFileStrategy::RetrieveChunkPosts(const std::string& entity,
                                        const std::string& post_id,
                                        ChunkPostList& out) {
    int status = ret::A_OK;
    std::cout<<" retrieving chunk posts ... " << std::endl;
    std::string posts_feed = GetConfigValue("posts_feed");
    UrlParams params;
    params.AddValue(std::string("mentions"), entity + "+" + post_id);
    params.AddValue(std::string("types"), std::string(cnst::g_attic_chunk_type));

    std::string prm;
    params.SerializeToString(prm);
    std::cout<<" RetrieveChunkPosts params : " << prm << std::endl;

    Response response;
    netlib::HttpGet(posts_feed,
                    &params,
                    &access_token_,
                    response);

    std::cout<<" CODE : " << response.code << std::endl;
    std::cout<<" BODY : " << response.body << std::endl;

    if(response.code == 200) {
        PagePost pp;
        jsn::DeserializeObject(&pp, response.body);
        Json::Value chunk_post_arr(Json::arrayValue);
        jsn::DeserializeJson(pp.data(), chunk_post_arr);

        std::cout<<" TOTAL POST COUNT : " << chunk_post_arr.size() << std::endl;
        Json::ValueIterator itr = chunk_post_arr.begin();
        for(; itr != chunk_post_arr.end(); itr++) {
            Post gp;
            jsn::DeserializeObject(&gp, (*itr));
            // There should never be more than one post in the same group
            std::cout<<" CHUNK POST TYPE : " << gp.type() << std::endl;
            if(gp.type().find(cnst::g_attic_chunk_type) != std::string::npos) {
                ChunkPost p;
                jsn::DeserializeObject(&p, (*itr));
                std::cout<<"TYPE : " << p.type() << std::endl;
                std::cout<<" PUSHING BACK GROUP : " << p.group() << std::endl;
                if(out.find(p.group()) == out.end()) {
                    out[p.group()] = p;
                    std::cout<<" CHUNK INFO LIST SIZE : " << p.chunk_info_list_size() << std::endl;
                }
                else 
                    std::cout<<" DUPLICATE GROUP CHUNK POST, RESOLVE " << std::endl;
            }
        }
    }
    else { 
        log::LogHttpResponse("FA332ASDF3", response);
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

void GetFileStrategy::GetMasterKey(std::string& out) {
    MasterKey mKey;
    credentials_manager_->GetMasterKeyCopy(mKey);
    mKey.GetMasterKey(out);
}

int GetFileStrategy::ExtractCredentials(FilePost& in, Credentials& out) {
    int status = ret::A_OK;
    
    std::string key, iv;
    key = in.key_data();
    iv = in.iv_data();

    std::string mk;
    GetMasterKey(mk);
    std::cout << "MASTER KEY : " << mk << std::endl;
    Credentials FileKeyCred;
    FileKeyCred.set_key(mk);
    FileKeyCred.set_iv(iv);

    // Decrypt File Key
    std::string filekey;
    //crypto::DecryptStringCFB(key, FileKeyCred, filekey);
    status = crypto::DecryptStringGCM(key, FileKeyCred, filekey);
    std::cout<<" ORIGINAL KEY : " << key << std::endl;
    std::cout<<" FILE KEY : " << filekey << std::endl;
    if(status == ret::A_OK) {
        out.set_key(filekey);
        out.set_iv(iv);
    }
    else { 
        std::cout<<" FAILED TO BUILD FILE KEY " << std::endl;
    }

    return status;
}

int GetFileStrategy::ConstructFile(ChunkPostList& chunk_posts, 
                                   const Credentials& file_cred, 
                                   FileInfo* fi) {
    int status = ret::A_OK;
    unsigned int post_count = chunk_posts.size();
    std::cout<<" # CHUNK POSTS : " << post_count << std::endl;

    if(post_count > 0) {
        std::string temp_path;
        GetTemporaryFilepath(fi, temp_path);

        std::ofstream ofs;
        ofs.open(temp_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
        if(ofs.is_open()) {
            std::string post_attachment = GetConfigValue("post_attachment");
            for(unsigned int i=0; i < post_count; i++) {
                if(chunk_posts.find(i) != chunk_posts.end()) {
                    ChunkPost cp = chunk_posts.find(i)->second;
                    unsigned int chunk_count = cp.chunk_info_list_size();
                    std::cout<<" CHUNK COUNT : " << chunk_count << std::endl;
                    for(unsigned int j=0; j<chunk_count; j++) {
                        ChunkPost::ChunkInfoList::iterator itr = cp.chunk_info_list()->find(j);
                        if(itr != cp.chunk_info_list()->end()) {
                            std::cout<<" FOUND CHUNK # : " << i << std::endl;
                            // Get attachment
                            if(cp.has_attachment(itr->second.chunk_name())) {
                                Attachment attch = cp.get_attachment(itr->second.chunk_name());
                                std::string attachment_path;
                                utils::FindAndReplace(post_attachment,
                                                      "{digest}",
                                                      attch.digest,
                                                      attachment_path);
                                std::cout<<" attachment path : " << attachment_path << std::endl;
                                std::string buffer;
                                status = RetrieveAttachment(attachment_path, buffer);
                                if(status == ret::A_OK) {
                                    std::cout<<" Fetching chunk : ... " << itr->second.chunk_name() << std::endl;
                                    //ChunkInfo* ci = fi->GetChunkInfo(itr->second.chunk_name());
                                    ChunkInfo* ci = cp.GetChunkInfo(itr->second.chunk_name());
                                    std::string chunk;
                                    status = TransformChunk(ci, file_cred.key(), buffer, chunk);
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
                    }
                }
                else {
                    std::cout<<" INVALID CHUNK POST " << std::endl;
                }
            }
            ofs.close();
        }
        if(status == ret::A_OK) {
            std::string path;
            file_manager_->GetCanonicalFilepath(fi->filepath(), path);
            fs::MoveFile(temp_path, path);
        }
        // delete temp file 
        fs::DeleteFile(temp_path);
    }

    return status;
}

bool GetFileStrategy::GetTemporaryFilepath(FileInfo* fi, std::string& out) { 
    std::string filename = fi->filename();

    std::string temp_path = file_manager_->temp_directory();
    if(fs::CheckFilepathExists(temp_path)) {
        utils::CheckUrlAndAppendTrailingSlash(temp_path);
        std::string randstr;
        utils::GenerateRandomString(randstr, 16);
        temp_path += filename + "_" + randstr;
        out = temp_path;
        return true;
    }
        
    return false;
}

int GetFileStrategy::RetrieveAttachment(const std::string& url, std::string& outBuffer) {
    int status = ret::A_OK;

    std::cout<<" RETRIEVING ATTACHEMNT : " << url << std::endl;

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
    else { 
        std::cout<<" INVALID CHUNK INFO PTR " << std::endl; 
        status = ret::A_FAIL_INVALID_PTR;
    }

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

bool GetFileStrategy::ValidMasterKey() {
    std::string mk;
    GetMasterKey(mk);
    if(mk.empty()) {
        std::string error = "Invalid master key, it is empty!";
        log::LogString("MASDP2823", error);
        return false;
    }
    return true;
}

}//namespace
