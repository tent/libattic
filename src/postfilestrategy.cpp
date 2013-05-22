#include "postfilestrategy.h"

#include <stdio.h>

#include "utils.h"
#include "compression.h"
#include "constants.h"
#include "fileinfo.h"
#include "filesystem.h"
#include "credentialsmanager.h"
#include "connectionhandler.h"

#include "rollsum.h"
#include "postutils.h"
#include "logutils.h"
#include "chunkbuffer.h"
#include "chunkrequest.h"
#include "pagepost.h"
#include "censushandler.h"
#include "chunktransform.h"
#include "posthandler.h"
#include "filehandler.h"

#include "sleep.h"

namespace attic { 

PostFileStrategy::PostFileStrategy() {}
PostFileStrategy::~PostFileStrategy() {}

int PostFileStrategy::Execute(FileManager* pFileManager,
                              CredentialsManager* pCredentialsManager) {
    int status = ret::A_OK;
    status = InitInstance(pFileManager, pCredentialsManager);
    // Initialize meta post
    post_path_ = GetConfigValue("post_path");
    posts_feed_ = GetConfigValue("posts_feed");
    std::string filepath = GetConfigValue("filepath");
    std::string entity = GetConfigValue("entity");

    std::cout<<" starting upload : " << filepath << std::endl;
    // Check Master Key before doing anything else
    if(!ValidMasterKey()) return ret::A_FAIL_INVALID_MASTERKEY;

    if(fs::CheckFilepathExists(filepath)) {
        FileInfo fi;
        if(!RetrieveFileInfo(filepath, fi))
            return ret::A_FAIL_INVALID_FILE_INFO;
        std::string file_post_id = fi.post_id();
        status = UpdateFilePostTransitState(file_post_id, true);

        std::cout<<" File Meta Data initialization status : " << status << std::endl;
        // Verify key credentials
        if(!fi.file_credentials().key_empty()) {
            std::cout<<" INITIALIZED META POST ID : "<< file_post_id << std::endl;
            if(status == ret::A_OK && !file_post_id.empty()) {
                // Retrieve Chunk posts
                ChunkPostList chunk_posts;
                RetrieveChunkPosts(entity, file_post_id, chunk_posts);
                // Extract Chunk info
                FileInfo::ChunkMap chunk_map;
                ExtractChunkInfo(chunk_posts, chunk_map);
                // begin chunking
                status = ChunkFile(filepath, fi.file_credentials(), file_post_id, chunk_posts, chunk_map);
                std::cout<<" CHUNK FILE STATUS : " << status << std::endl;

                if(status == ret::A_OK) { 
                    // Update file info
                    file_manager_->SetFileChunks(fi.filepath(), chunk_map);
                    // Update meta data transit state
                    status = UpdateFilePostTransitState(file_post_id, false);

                    // Last thing that should happen
                    if(!UpdateFilePostVersion(&fi, file_post_id)){
                        std::string error = " Failed to update file post version";
                        log::LogString("MASDF@12934", error);
                    }
                }
                else {
                    // TODO :: Undo the file to the last good version, or delete if no last good version

                }
            }
            else if(status == ret::A_OK && file_post_id.empty()) {
                std::cout<<" META POST ID EMPTY " << std::endl;
                status = ret::A_FAIL_INVALID_POST_ID;
            }
        }
        else {
            std::string error = "Invalid file key during post file ";
            log::LogString("KJASDmmm++234", error);
            status = ret::A_FAIL_INVALID_FILE_KEY;
        }
    }
    else {
        status = ret::A_FAIL_PATH_DOESNT_EXIST;
    }

    std::cout<<" finishing upload : " << filepath <<" status : " << status << std::endl;
    return status;
}

int PostFileStrategy::RetrieveChunkPosts(const std::string& entity,
                                         const std::string& post_id,
                                         ChunkPostList& out) {
    int status = ret::A_OK;
    std::string posts_feed = GetConfigValue("posts_feed");
    UrlParams params;
    params.AddValue(std::string("mentions"), entity + "+" + post_id);
    params.AddValue(std::string("type"), std::string(cnst::g_attic_chunk_type));

    Response response;

    //ConnectionHandler ch;
    netlib::HttpGet(posts_feed,
               &params,
               &access_token_,
               response);

    if(response.code == 200) {
        PagePost pp;
        jsn::DeserializeObject(&pp, response.body);

        Json::Value chunk_post_arr(Json::arrayValue);
        jsn::DeserializeJson(pp.data(), chunk_post_arr);
        Json::ValueIterator itr = chunk_post_arr.begin();
        for(; itr != chunk_post_arr.end(); itr++) {
            ChunkPost p;
            jsn::DeserializeObject(&p, (*itr));
            // There should never be more than one post in the same group
            if(out.find(p.group()) == out.end())
                out[p.group()] = p;
            else 
                std::cout<<" DUPLICATE GROUP CHUNL POST, RESOLVE " << std::endl;
        }
    }
    else { 
        log::LogHttpResponse("FA332JMA3", response);
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

void PostFileStrategy::ExtractChunkInfo(ChunkPostList& list,
                                        FileInfo::ChunkMap& out) {
    ChunkPostList::iterator itr = list.begin();

    for(;itr != list.end(); itr++) {
        ChunkPost::ChunkInfoList::iterator cp_itr = itr->second.chunk_info_list()->begin();
        for(;cp_itr != itr->second.chunk_info_list()->end(); cp_itr++) {
            ChunkInfo ci = cp_itr->second;

            if(ci.group() == -1) { // debug
                std::cout<<" CHUNK INFO GROUP NOT SET " << std::endl;
                ci.set_group(itr->first);
            }

            out[ci.chunk_name()] = ci;
        }
    }
}

int PostFileStrategy::ChunkFile(const std::string& filepath,
                                const Credentials& file_credentials,
                                const std::string& file_post_id,
                                ChunkPostList& chunk_list,
                                FileInfo::ChunkMap& chunk_map) {
    // To Chunk a file
    //  - Chunk and upload sequentially
    //  - Check if a chunk group(post) exists, if it does update that post, else new post
    int status = ret::A_OK;
    ChunkBuffer cb;
    if(cb.OpenFile(filepath)) {
        std::string file_key = file_credentials.key();
        unsigned int total_read = 0;
        unsigned int chunk_count = 0;
        unsigned int group_count = 0;
        std::string chunk;
        bool new_group = false;
        ChunkPost* cp = NULL;

        std::string entity = GetConfigValue("entity");
        ChunkRequest* cr = NULL;
        while(cb.ReadChunk(chunk)) {
            // Find Chunk Group
            if(chunk_count == 0) {
                cr = new ChunkRequest(entity, 
                                      posts_feed_, 
                                      post_path_, 
                                      file_post_id, 
                                      access_token_, 
                                      group_count);
                // Begin new chunk post
                if(chunk_list.find(group_count) != chunk_list.end()){
                    ChunkPost parent = chunk_list.find(group_count)->second;
                    cr->set_parent_post(parent);
                }
                else {
                    new_group = true;
                }

                cr->BeginRequest();
            }
            // Transform chunk
            ChunkInfo ci;
            ChunkTransform ct(chunk, file_key);
            cr->ProcessTransform(ct, chunk_count, ci);
            verification_map_[ci.verification_hash()] = true;
            chunk_map[ci.chunk_name()] = ci;
            
            chunk_count++;
            if(cb.BufferEmpty() || chunk_count >= 30) {
                // End chunk post
                chunk_count = 0;
                group_count++;

                Response response;
                cr->EndRequest(response);
                if(cr) {
                    delete cr;
                    cr = NULL;
                }
                if(response.code == 200) { 
                    // Verifiy chunks made it to the server
                    std::cout<<" cr response : " << response.body << std::endl;
                    ChunkPost cp;
                    jsn::DeserializeObject(&cp, response.body);
                    if(!VerifyChunks(cp, filepath)) {
                        status = ret::A_FAIL_ATTACHMENT_VERIFICATION;
                        break;
                    }
                }
                else {
                    std::cout<<" CHUNK POST FAILED AT GROUP : " << group_count << std::endl;
                    log::LogHttpResponse("KJASDF321", response);
                }
            }
            chunk.clear();
        }
    }

    return status;
}

bool PostFileStrategy::VerifyChunks(ChunkPost& cp, const std::string& filepath) {
    std::cout<<" verification map size : " << verification_map_.size() << std::endl;
    Post::AttachmentMap::iterator itr_cp = cp.attachments()->begin();
    for(;itr_cp != cp.attachments()->end(); itr_cp++) {
        std::cout<< itr_cp->second.digest << std::endl;
        std::string decoded;
        if(verification_map_.find(itr_cp->second.digest) == verification_map_.end()){
            std::string error = "Failed to validate attachment integrity.\n";
            error += "\t filepath : " + filepath + "\n";
            error += "\t attachment name : " + itr_cp->second.name + "\n";
            char buf[256] = {'\0'};
            snprintf(buf, 256, "%u", itr_cp->second.size);
            error += "\t size : " + std::string(buf) + "\n";
            log::LogString("MAS021n124", error);
            return false;
        }
        else {
            std::cout<<" ATTACHMENT DIGEST FOUND " << std::endl;
        }
    }
    return true;
}

int PostFileStrategy::TransformChunk(const std::string& chunk, 
                                     const std::string& fileKey,
                                     std::string& finalizedOut, 
                                     std::string& nameOut, 
                                     ChunkInfo& out) {
    int status = ret::A_OK;
    Credentials chunkCred;
    chunkCred.set_key(fileKey);

    // Calculate plaintext hash
    std::string plaintextHash;
    crypto::GenerateHash(chunk, plaintextHash);
    std::cout<<" PLAINTEXT : " << plaintextHash << std::endl;

    // create chunk name (hex encoded plaintext hash)
    std::string chunkName;
    utils::StringToHex(plaintextHash, chunkName);

    // Compress
    std::string compressedChunk;
    compress::CompressString(chunk, compressedChunk);

    std::string compressedHash;
    crypto::GenerateHash(compressedChunk, compressedHash);
    std::cout<<" COMPRESSED HASH : " << compressedHash << std::endl;

    // Encrypt
    std::string encryptedChunk;
    std::string iv;
    crypto::GenerateIv(iv);
    chunkCred.set_iv(iv);

    std::cout<<" FILE KEY : " << fileKey << std::endl;
    std::cout<< " IV : " << iv << std::endl;
    //crypto::EncryptStringCFB(compressedChunk, chunkCred, encryptedChunk);
    crypto::EncryptStringGCM(compressedChunk, chunkCred, encryptedChunk);

    // Base64 Encode
    std::string finishedChunk;
    crypto::Base64EncodeString(encryptedChunk, finishedChunk);
    finalizedOut = finishedChunk;

    std::string ciphertextHash;
    crypto::GenerateHash(encryptedChunk, ciphertextHash);
    std::cout<<" CIPHER TEXT : " << ciphertextHash << std::endl;

    // Verification Hash, has of base64 encoded chunk, used to verify each chunk is 
    // successfully uplaoded
    std::string ver;
    crypto::GenerateHexEncodedHmac(finalizedOut, ver);
    std::cout<<" VERIFICATION PRE : " << ver << std::endl;
    ver = ver.substr(0, 64);
    std::cout<<" VERIFICATION HASH : " << ver << std::endl;
    verification_map_[ver] = true;

    // Fill Out Chunk info object
    out.set_chunk_name(chunkName);
    out.set_plaintext_mac(plaintextHash);
    out.set_ciphertext_mac(ciphertextHash);
    out.set_iv(iv);

    nameOut = chunkName;

    return status;
}

void PostFileStrategy::GetMasterKey(std::string& out) {
    MasterKey mKey;
    credentials_manager_->GetMasterKeyCopy(mKey);
    mKey.GetMasterKey(out);
}

void PostFileStrategy::UpdateFileInfo(const Credentials& fileCred, 
                                      const std::string& filepath, 
                                      const std::string& chunkpostid,
                                      const std::string& post_version,
                                      FileInfo* fi) {
    std::string filename;
    utils::ExtractFileName(filepath, filename);
    unsigned int filesize = utils::CheckFilesize(filepath);

    // Encrypt File Key
    std::string mk;
    GetMasterKey(mk);

    fi->set_post_version(post_version);

    // Insert File Data
    fi->set_chunk_post_id(chunkpostid);
    fi->set_filepath(filepath);
    fi->set_filename(filename);
    fi->set_file_size(filesize);
    fi->set_file_credentials(fileCred);

    // Encrypt File Key
    std::string fileKey = fileCred.key();
    std::string fileIv = fileCred.iv();

    std::cout<<" Update file info file key : " << fileKey << std::endl;
    std::cout<<" Update file info file iv : " << fileIv << std::endl;

    Credentials fCred;
    fCred.set_key(mk);
    fCred.set_iv(fileIv);

    std::string encryptedKey;
    //crypto::EncryptStringCFB(fileKey, fCred, encryptedKey);
    crypto::EncryptStringGCM(fileKey, fCred, encryptedKey);

    std::cout<<" Update file ENCRYPTED file key : " << encryptedKey << std::endl;
    std::string copy_test;
    copy_test = encryptedKey;
    std::cout<<" COPY file ENCRYPTED file key : " << copy_test << std::endl;
    if(encryptedKey == copy_test) { std::cout<<" KEYS ARE THE SAME ! " << std::endl; } else { std::cout<<"KEYS ARE DIFFERENT " << std::endl; }

    fi->set_file_credentials_iv(fileIv);
    fi->set_file_credentials_key(fileKey);
    fi->set_encrypted_key(encryptedKey);

    // Insert file info to manifest
    file_manager_->InsertToManifest(fi);
}

bool PostFileStrategy::RetrieveFileInfo(const std::string& filepath, FileInfo& out) {
    bool ret = false;
    FileHandler fh(file_manager_);
    if(!fh.DoesFileExist(filepath)) {
        if(fh.CreateNewFile(filepath, out)) {
            FilePost fp;
            fp.InitializeFilePost(&out, false);
            PostHandler<FilePost> ph(access_token_);
            Response response;
            if(ph.Post(posts_feed_, NULL, fp, response) == ret::A_OK) {
                Post post;
                jsn::DeserializeObject(&post, response.body);
                out.set_post_id(post.id());
                fh.UpdateFilePostId(out.filepath(), post.id());
                return true;
            }
        }
    }
    return false;
}

int PostFileStrategy::UpdateFilePostTransitState(const std::string& post_id, bool in_transit) {
    int status = ret::A_OK;
    if(!post_id.empty()) {
        std::string posturl;
        utils::FindAndReplace(post_path_, "{post}", post_id, posturl);

        PostHandler<FilePost> ph(access_token_);
        FilePost p;
        Response get_resp;
        status = ph.Get(posturl, NULL, p, get_resp);

        if(status == ret::A_OK) {
            p.clear_fragment();
            if(in_transit)
                p.set_fragment(cnst::g_transit_fragment);

            Response put_resp;
            status = ph.Put(posturl, NULL, p, put_resp);
            if(status != ret::A_OK) {
                status = ret::A_FAIL_NON_200;
                log::LogHttpResponse("PO1090MASDF", put_resp);
            }
        }
        else {
            status = ret::A_FAIL_NON_200;
            log::LogHttpResponse("nmasd981", get_resp);
        }

    }
    else {
        status = ret::A_FAIL_INVALID_POST_ID;
    }
    return status;
}

bool PostFileStrategy::UpdateFilePostVersion(const FileInfo* fi, const std::string& meta_post_id) {
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", meta_post_id, posturl);

    // Get Existing post
    Response get_resp;
    ConnectionHandler ch;
    netlib::HttpGet(posturl,
               NULL,
               &access_token_,
               get_resp);

    if(get_resp.code == 200) {
        FilePost p;
        jsn::DeserializeObject(&p, get_resp.body);
        file_manager_->SetFileVersion(fi->filepath(), p.version()->id());
        return true;
    }
    return false;
}

bool PostFileStrategy::RetrieveFolderPostId(const std::string& filepath, std::string& id_out) {
    // Get Parent folder
    std::string folderpath;
    fs::GetParentPath(filepath, folderpath);

    bool ret = false;
    Folder folder;
    if(file_manager_->GetFolderEntry(folderpath, folder)) {
        id_out = folder.folder_post_id();
        ret = true;
    }
    return ret;
}

int PostFileStrategy::ExtractCredentials(FilePost& in, Credentials& out) {
    int status = ret::A_OK;
    
    std::string key, iv;
    key = in.key_data();
    iv = in.iv_data();

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

    return status;
}

bool PostFileStrategy::ValidMasterKey() {
    std::string mk;
    GetMasterKey(mk);
    if(mk.empty()) {
        std::string error = "Invalid master key, it is empty!";
        log::LogString("MASDF1000", error);
        return false;
    }
    return true;
}

}//namespace

