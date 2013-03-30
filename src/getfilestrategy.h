#ifndef GETFILESTRATEGY_H_
#define GETFILESTRATEGY_H_
#pragma once

#include "httpstrategy.h"
#include "credentials.h"
#include "post.h"

namespace attic { 

class GetFileStrategy : public HttpStrategyInterface {
    int RetrieveFileCredentials(FileInfo* fi, Credentials& out);
    int GetChunkPost(FileInfo* fi, Response& responseOut);
    int RetrieveFile(const std::string& filepath, 
                     const std::string& postpath, 
                     const Credentials& fileCred,
                     Post& post,
                     FileInfo* fi);

    int RetrieveAttachment(const std::string& url, std::string& outBuffer);
    int TransformChunk(const ChunkInfo* ci, 
                       const std::string& filekey,
                       const std::string& chunkBuffer, 
                       std::string& out);
public:
    GetFileStrategy();
    ~GetFileStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager,
                Response& out);
};

}//namespace
#endif

