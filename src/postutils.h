#ifndef POSTUTILS_H_
#define POSTUTILS_H_
#pragma once

#include <vector>
#include <string>

#include "jsonserializable.h"
#include "post.h"
#include "filepost.h"
#include "chunkpost.h"
#include "response.h"
#include "constants.h"

namespace attic { namespace postutils {

static int InitializeFilePost(FileInfo* pFi, FilePost& postOut, bool isPublic);
static int DeserializeFilePostIntoFileInfo(const FilePost& post, FileInfo& fiOut);
static int DeserializePostIntoFileInfo(const Post* post, FileInfo& fiOut);
static int DeserializePostIntoFileInfo(const Post& post, FileInfo& fiOut);
static int ExtractChunkPostsFromResponse(const Response& response, std::vector<ChunkPost>* out);
static void ConstructPostUrl(const std::string& apiroot, std::string& out);

static void ConstructPostUrl(const std::string& apiroot, std::string& out) { // Depricated
    std::cout<<" REDO ConstructPostUrl " << std::endl;
    out = apiroot;
    utils::CheckUrlAndAppendTrailingSlash(out);
    //out += cnst::g_szPostEndpoint;
}

static int InitializeFilePost(FileInfo* pFi, FilePost& postOut, bool isPublic) {
    int status = ret::A_OK;
    if(pFi) {
        std::string filepath = pFi->filepath();
        std::string filename = pFi->filename();
        unsigned int size = pFi->file_size();

        // Set Basic attic post info
        postOut.set_public(isPublic);
        postOut.set_relative_path(filepath);
        postOut.set_name(filename);
        postOut.set_file_size(size);
        postOut.set_deleted(pFi->deleted());

        // Set Attic post key info
        std::string encryptedkey = pFi->encrypted_key();
        std::string iv = pFi->file_credentials_iv();

        postOut.set_key_data(encryptedkey);
        postOut.set_iv_data(iv);
        
        // Set Chunk info
        std::string chunkpostid = pFi->chunk_post_id();

        postOut.PushBackChunkPostId(chunkpostid);

        FileInfo::ChunkMap* pList = pFi->GetChunkInfoList();

        if(pList) {
            std::cout<<" CHUNK LIST SIZE : " << pList->size() << std::endl;
            FileInfo::ChunkMap::iterator itr = pList->begin();

            std::string identifier;
            for(;itr != pList->end(); itr++) {
                identifier.clear();
                identifier = itr->second.checksum();
                postOut.PushBackChunkIdentifier(identifier);
            }
        }
        else {
            std::cout<<" INVALID CHUNK LIST : " << std::endl;
        }

    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

static int DeserializeFilePostIntoFileInfo(const FilePost& post, FileInfo& fiOut) {
    int status = ret::A_OK;
    
    // Attic Post specific
    std::string name = post.name();
    std::string path = post.relative_path();
    std::string key = post.key_data();
    std::string iv = post.iv_data();

    fiOut.set_filename(name);
    fiOut.set_filepath(path);
    fiOut.set_encrypted_key(key);
    fiOut.set_file_credentials_iv(iv);

    status = DeserializePostIntoFileInfo(post, fiOut);
    return status;
}

static int DeserializePostIntoFileInfo(const Post& post, FileInfo& fiOut) {
    return DeserializePostIntoFileInfo(&post, fiOut);
}

static int DeserializePostIntoFileInfo(const Post* post, FileInfo& fiOut) {
    int status = ret::A_OK;

    if(post) {
        std::string postid = post->id();
        fiOut.set_post_id(postid);
        std::cout<<" IMPLEMENT DESERIALIZEPOSTINTOFILEINFO VO3" << std::endl;
        //TODO V03
        //fiOut.SetPostVersion(post->GetVersion());
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

static int ExtractChunkPostsFromResponse(const Response& response, std::vector<ChunkPost>* out) {
    int status = ret::A_OK;

    if(out) {
        Json::Value val;
        std::string input = response.body;
        jsn::DeserializeJsonValue(val, input);

        Json::ValueIterator itr = val.begin();
        for(;itr != val.end(); itr++) {
            ChunkPost post;
            jsn::DeserializeObject(&post, *itr);
            out->push_back(post);
        }
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

}}//namespace
#endif


