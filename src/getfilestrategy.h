#ifndef GETFILESTRATEGY_H_
#define GETFILESTRATEGY_H_
#pragma once

#include <string>
#include <map>

#include "httpstrategy.h"
#include "credentials.h"
#include "chunkpost.h"
#include "posttree.h"
#include "filepost.h"

namespace attic { 

class GetFileStrategy : public HttpStrategyInterface {
    typedef std::map<unsigned int, ChunkPost> ChunkPostList; // key, chunk group

    int RetrieveAttachment(const std::string& url, std::string& outBuffer);
    int TransformChunk(const ChunkInfo* ci, 
                       const std::string& filekey,
                       const std::string& chunkBuffer, 
                       std::string& out);
    int ConstructPostTree(FileInfo* fi, PostTree& tree);
    int RetrieveAndInsert(const std::string& postid, PostTree& tree);

    int RetrieveFilePost(const std::string& post_id, FilePost& out);


    int RetrieveChunkPosts(const std::string& entity,
                           const std::string& post_id,
                           ChunkPostList& out);
    int ExtractCredentials(FilePost& in, Credentials& out);
    void ConstructFilepath(const FileInfo& fi, const Folder& folder, std::string& out);
    int ConstructFile(ChunkPostList& chunk_posts, 
                      const Credentials& file_cred, 
                      FileInfo& fi,
                      const std::string& destination_path);
    void GetMasterKey(std::string& out);
    bool ValidMasterKey();

    void ValidateFolderEntries(FilePost& fp);
    void RetrieveFolderPosts(FilePost& fp, std::deque<FolderPost>& out);
    bool RetrieveFolderPost(const std::string& post_id, FolderPost& out);
public:
    GetFileStrategy();
    ~GetFileStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);
};

}//namespace
#endif

