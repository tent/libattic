#ifndef GETFILESTRATEGY_H_
#define GETFILESTRATEGY_H_
#pragma once

#include <map>

#include "httpstrategy.h"
#include "credentials.h"
#include "chunkpost.h"
#include "posttree.h"
#include "filepost.h"

namespace attic { 

class GetFileStrategy : public HttpStrategyInterface {
    typedef std::map<unsigned int, ChunkPost> ChunkPostList; // key, chunk group

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
    int RetrieveChunkPosts(const std::string& entity,
                           const std::string& post_id,
                           ChunkPostList& out);
    int ExtractCredentials(FilePost& in, Credentials& out);
    int ConstructFile(ChunkPostList& chunk_posts, 
                      const Credentials& file_cred, 
                      FileInfo* fi);
    bool GetTemporaryFilepath(FileInfo* fi, std::string& out);
public:
    GetFileStrategy();
    ~GetFileStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);
};

}//namespace
#endif

