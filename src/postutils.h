#ifndef POSTUTILS_H_
#define POSTUTILS_H_
#pragma once

#include <vector>
#include <string>

#include "jsonserializable.h"
#include "post.h"
#include "atticpost.h"
#include "chunkpost.h"
#include "response.h"
#include "constants.h"

namespace postutils {

static int InitializeAtticPost(FileInfo* pFi, AtticPost& postOut, bool isPublic);
static int DeserializeAtticPostIntoFileInfo(const AtticPost& post, FileInfo& fiOut);
static int DeserializePostIntoFileInfo(const Post* post, FileInfo& fiOut);
static int DeserializePostIntoFileInfo(const Post& post, FileInfo& fiOut);
static int ExtractChunkPostsFromResponse(const Response& response, std::vector<ChunkPost>* out);
static void ConstructPostUrl(const std::string& apiroot, std::string& out);

static void ConstructPostUrl(const std::string& apiroot, std::string& out) {
    out = apiroot;
    utils::CheckUrlAndAppendTrailingSlash(out);
    out += cnst::g_szPostEndpoint;
}

static int InitializeAtticPost(FileInfo* pFi, AtticPost& postOut, bool isPublic) {
    int status = ret::A_OK;
    if(pFi) {
        std::string filepath, filename;
        pFi->GetFilepath(filepath);
        pFi->GetFilename(filename);
        unsigned int size = pFi->GetFileSize();

        // Set Basic attic post info
        postOut.SetPublic(isPublic);
        postOut.AtticPostSetFilepath(filepath);
        postOut.AtticPostSetFilename(filename);
        postOut.AtticPostSetSize(size);
        postOut.AtticPostSetDeleted(pFi->GetDeleted());

        // Set Attic post key info
        std::string encryptedkey, iv;
        pFi->GetEncryptedKey(encryptedkey);
        pFi->GetIv(iv);

        postOut.AtticPostSetKeyData(encryptedkey);
        postOut.AtticPostSetIvData(iv);
        
        // Set Chunk info
        std::string chunkpostid;
        pFi->GetChunkPostID(chunkpostid);

        postOut.PushBackChunkPostId(chunkpostid);

        FileInfo::ChunkMap* pList = pFi->GetChunkInfoList();

        if(pList) {
            std::cout<<" CHUNK LIST SIZE : " << pList->size() << std::endl;
            FileInfo::ChunkMap::iterator itr = pList->begin();

            std::string identifier;
            for(;itr != pList->end(); itr++) {
                identifier.clear();
                itr->second.GetChecksum(identifier);
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

static int DeserializeAtticPostIntoFileInfo(const AtticPost& post, FileInfo& fiOut) {
    int status = ret::A_OK;
    
    // Attic Post specific
    std::string name, path, chunkname, key, iv;
    post.GetAtticPostFilename(name);
    post.GetAtticPostFilepath(path);
    post.GetAtticPostChunkName(chunkname);
    post.GetAtticPostKeyData(key);
    post.GetAtticPostIvData(iv);

    fiOut.SetFilename(name);
    fiOut.SetFilepath(path);
    fiOut.SetChunkName(chunkname);
    fiOut.SetEncryptedKey(key);
    fiOut.SetIv(iv);

    status = DeserializePostIntoFileInfo(post, fiOut);
    return status;
}

static int DeserializePostIntoFileInfo(const Post& post, FileInfo& fiOut) {
    return DeserializePostIntoFileInfo(&post, fiOut);
}

static int DeserializePostIntoFileInfo(const Post* post, FileInfo& fiOut) {
    int status = ret::A_OK;

    if(post) {
        std::string postid;
        post->GetID(postid);

        fiOut.SetPostID(postid);
        fiOut.SetPostVersion(post->GetVersion());
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

};
#endif


