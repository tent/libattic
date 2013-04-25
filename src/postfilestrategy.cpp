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

    if(fs::CheckFilepathExists(filepath)) {
        FileInfo* fi = RetrieveFileInfo(filepath); // null check in method call
        std::string meta_post_id;
        std::cout<<" Initializing File Meta Data " << std::endl;
        status = InitializeFileMetaData(fi, filepath, meta_post_id);
        if(status == ret::A_OK) {
            // Retrieve Chunk posts
            ChunkPostList chunk_posts;
            RetrieveChunkPosts(entity, meta_post_id, chunk_posts);
            // Extract Chunk info
            FileInfo::ChunkMap chunk_map;
            ExtractChunkInfo(chunk_posts, chunk_map);
            // begin chunking
            status = ChunkFile(filepath, fi->file_credentials(), meta_post_id, chunk_posts, chunk_map);
            std::cout<<" CHUNK FILE STATUS : " << status << std::endl;

            if(status == ret::A_OK) { 
                // Update file info
                file_manager_->SetFileChunks(fi->filepath(), chunk_map);
                // Update meta data transit state
                status = UpdateFilePostTransitState(meta_post_id, false);
            }
        }
    }
    else {
        status = ret::A_FAIL_PATH_DOESNT_EXIST;
    }

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

    ConnectionHandler ch;
    ch.HttpGet(posts_feed,
               &params,
               &access_token_,
               response);

    if(response.code == 200) {
        Json::Value chunk_post_arr(Json::arrayValue);
        jsn::DeserializeJson(response.body, chunk_post_arr);
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
                                const std::string& file_meta_post_id,
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
                                      file_meta_post_id, 
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
            std::string finished_chunk, chunk_name;
            ChunkInfo ci;
            TransformChunk(chunk, 
                           file_key, 
                           finished_chunk, 
                           chunk_name, 
                           ci);
            ci.set_position(chunk_count);
            chunk_map[ci.chunk_name()] = ci;

            cr->PushBackChunk(ci, chunk_name, finished_chunk, chunk_count);
            
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
                std::cout<<" CHUNK ENDING " << std::endl;
                std::cout<<" code : " << response.code << std::endl;
                std::cout<<" response : " << response.body << std::endl;

                if(response.code != 200) { 
                    std::cout<<" CHUNK POST FAILED AT GROUP : " << group_count << std::endl;
                    log::LogHttpResponse("KJASDF321", response);
                }

            }
            chunk.clear();
        }
    }

    return status;
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

    // Fill Out Chunk info object
    out.set_chunk_name(chunkName);
    out.set_plaintext_mac(plaintextHash);
    out.set_ciphertext_mac(ciphertextHash);
    out.set_iv(iv);

    nameOut = chunkName;

    return status;
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
    MasterKey mKey;
    credentials_manager_->GetMasterKeyCopy(mKey);

    std::string mk;
    mKey.GetMasterKey(mk);

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
    std::cout<<" INSERTING TO MANIFEST " << std::endl;
    file_manager_->InsertToManifest(fi);
}

FileInfo* PostFileStrategy::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = file_manager_->GetFileInfo(filepath);
    if(!fi) { 
        fi = file_manager_->CreateFileInfo();
        // Get folder id
        std::string folderpath;
        if(fs::GetParentPath(filepath, folderpath) == ret::A_OK) {
            std::string folderid;
            file_manager_->GetFolderManifestId(folderpath, folderid);
            fi->set_folder_manifest_id(folderid);
        }
    }
    return fi;
}

int PostFileStrategy::InitializeFileMetaData(FileInfo* fi, 
                                             const std::string& filepath,
                                             std::string& post_id_out) {
    // If file info doesn't have a post, make sure it's created
    int status = ret::A_OK;
    // Setup File Meta Data
    std::string meta_data_post_id = fi->post_id();

    if(meta_data_post_id.empty()) {
        // Get Folder post id
        std::string folder_post_id;
        if(RetrieveFolderPostId(filepath, folder_post_id)) {
            std::string entity = GetConfigValue("entity");

            Credentials file_cred;
            crypto::GenerateCredentials(file_cred);
            UpdateFileInfo(file_cred, filepath, "", "", fi);

            std::string posturl = posts_feed_;
            // New Post
            FilePost p;
            p.InitializeFilePost(fi, false);
            p.MentionPost(entity, folder_post_id);
            p.set_in_transit(true);
            std::string post_buffer;
            jsn::SerializeObject(&p, post_buffer);

            Response response;
            ConnectionHandler ch;
            status = ch.HttpPost(posturl,
                                      p.type(),
                                      NULL,
                                      post_buffer,
                                      &access_token_,
                                      response);
            if(response.code == 200) {
                std::cout<<" FILE INITIALIZED : " << response.body << std::endl;
                Post post;
                jsn::DeserializeObject(&post, response.body);
                file_manager_->SetFilePostId(filepath, post.id());

                post_id_out = post.id();
                fi->set_post_id(post.id());
                
                FileInfo* ffi = RetrieveFileInfo(filepath);
                std::cout<<"encrypted key : " << ffi->encrypted_key() << std::endl;

                UpdateFileInfo(file_cred, filepath, "", "", fi);
            }
            else {
                status = ret::A_FAIL_NON_200;
            }
        }
        else {
            status = ret::A_FAIL_INVALID_FOLDER_POST;
        }
    }
    else {
        status = UpdateFilePostTransitState(meta_data_post_id, true);
    }

    return status;
}

int PostFileStrategy::UpdateFilePostTransitState(const std::string& post_id, bool in_transit) {
    int status = ret::A_OK;
    std::cout<<" UPDATE TRANSIT STATE " << std::endl;
    std::string posturl;
    utils::FindAndReplace(post_path_, "{post}", post_id, posturl);
    std::cout<<" PUST URL : " << posturl << std::endl;
    FilePost p;
    // Get Existing post
    Response get_resp;
    ConnectionHandler ch;
    ch.HttpGet(posturl,
               NULL,
               &access_token_,
               get_resp);

    std::cout<<" CODE : " << get_resp.code << std::endl;
    std::cout<<" BODY : " << get_resp.body << std::endl;
    if(get_resp.code == 200) {
        jsn::DeserializeObject(&p, get_resp.body);
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    std::cout<<" GOT EXISTING POST : " << status << std::endl;

    if(status == ret::A_OK) {
        Parent parent;
        parent.version = p.version()->id;
        // Set its transit state
        p.set_in_transit(in_transit);
        p.PushBackParent(parent);
        // Put
        std::string put_buffer;
        jsn::SerializeObject(&p, put_buffer);

        Response put_resp;
        ConnectionHandler ch;
        status = ch.HttpPut(posturl,
                                 p.type(),
                                 NULL,
                                 put_buffer,
                                 &access_token_,
                                 put_resp);
        if(put_resp.code == 200) {
            std::cout<<" success : " << put_resp.body << std::endl;
        }
        else {
            status = ret::A_FAIL_NON_200;
        }

        std::cout<<" PUT UPDATED POST " << std::endl;
        std::cout<<" CODE : " << put_resp.code << std::endl;
        std::cout<<" BODY : " << put_resp.body << std::endl;
    }

    return status;
}

bool PostFileStrategy::RetrieveFolderPostId(const std::string& filepath, std::string& id_out) {
    std::cout<<" retrieving folder post id " << std::endl;
    std::cout<<" filepath : " << filepath << std::endl;
    // Get Parent folder
    std::string folderpath;
    fs::GetParentPath(filepath, folderpath);

    bool ret = false;
    Folder folder;
    if(file_manager_->GetFolderEntry(folderpath, folder)) {
        id_out = folder.folder_post_id();
        ret = true;
    }
    std::cout<<" folder post id : " << id_out << std::endl;
    return ret;
}

}//namespace

