#ifndef GETFILESTRATEGY_H_
#define GETFILESTRATEGY_H_
#pragma once

#include "httpstrategy.h"
#include "credentials.h"
#include "chunkpost.h"
#include "posttree.h"
#include "filepost.h"

namespace attic { 

class GetFileStrategy : public HttpStrategyInterface {
    int RetrieveFileCredentials(FileInfo* fi, Credentials& out);
    int GetChunkPost(FileInfo* fi, Response& responseOut);

    int AssembleFile(const std::string& filepath,
                     const std::string& url,
                     const Credentials& file_cred,
                     ChunkPost& post,
                     FileInfo* fi);
                     
    int RetrieveAttachments(const std::string& filepath,
                            const std::string& attacment_url,
                            const Credentials& cred,
                            ChunkPost& post,
                            FileInfo* fi);

    int RetrieveAttachment(const std::string& url, std::string& outBuffer);
    int TransformChunk(const ChunkInfo* ci, 
                       const std::string& filekey,
                       const std::string& chunkBuffer, 
                       std::string& out);
    int ConstructPostTree(FileInfo* fi, PostTree& tree);
    int RetrieveAndInsert(const std::string& postid, PostTree& tree);

    int RetrieveFilePost(FileInfo* fi, FilePost& out);
public:
    GetFileStrategy();
    ~GetFileStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);
};

}//namespace
#endif

