#ifndef POSTUTILS_H_
#define POSTUTILS_H_
#pragma once

#include <string>

#include "atticpost.h"

namespace postutils
{
    static int DeserializePostIntoFileInfo(const Post* post, FileInfo& fiOut);
    static int DeserializePostIntoFileInfo(const Post& post, FileInfo& fiOut);

    static int InitializeAtticPostToFileInfo(FileInfo* pFi, AtticPost& postOut, bool isPublic)
    {
        int status = ret::A_OK;
        if(pFi)
        {
            std::string filepath, filename;
            pFi->GetFilepath(filepath);
            pFi->GetFilename(filename);
            unsigned int size = pFi->GetFileSize();

            // Set Basic attic post info
            postOut.SetPermission(std::string("public"), isPublic);
            postOut.AtticPostSetFilepath(filepath);
            postOut.AtticPostSetFilename(filename);
            postOut.AtticPostSetSize(size);

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
            if(pList)
            {
                FileInfo::ChunkMap::iterator itr = pList->begin();

                std::string identifier;
                for(;itr != pList->end(); itr++)
                {
                    identifier.clear();
                    itr->second.GetChecksum(identifier);
                    postOut.PushBackChunkIdentifier(identifier);
                }
            }
        }
        else
        {
            status = ret::A_FAIL_INVALID_PTR;
        }

        return status;
    }

    static int DeserializeAtticPostIntoFileInfo(const AtticPost& post, FileInfo& fiOut)
    {
        int status = ret::A_OK;
        
        // Attic Post specific
        std::string name, path, key, iv;
        post.GetAtticPostFilename(name);
        post.GetAtticPostFilepath(path);
        post.GetAtticPostKeyData(key);
        post.GetAtticPostIvData(iv);

        fiOut.SetFilename(name);
        fiOut.SetFilepath(path);
        fiOut.SetEncryptedKey(key);
        fiOut.SetIv(iv);

        status = DeserializePostIntoFileInfo(post, fiOut);
        return status;
    }

    static int DeserializePostIntoFileInfo(const Post& post, FileInfo& fiOut)
    {
        return DeserializePostIntoFileInfo(&post, fiOut);
    }

    static int DeserializePostIntoFileInfo(const Post* post, FileInfo& fiOut)
    {
        int status = ret::A_OK;

        if(post)
        {
            std::string postid;
            post->GetID(postid);

            fiOut->SetPostID(postid);
        }
        else
        {
            status = ret::A_FAIL_INVALID_PTR;
        }

        return status;
    }


};

#endif

