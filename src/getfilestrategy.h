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
#include "folderpost.h"
#include "folder.h"

namespace attic { 

class GetFileStrategy : public HttpStrategyInterface {
    typedef std::map<unsigned int, ChunkPost> ChunkPostList; // key, chunk group

    int RetrieveAttachment(const std::string& url, std::string& outBuffer);
    int TransformChunk(const ChunkInfo* ci, 
                       const std::string& filekey,
                       const std::string& chunkBuffer, 
                       std::string& out);

    int RetrieveFilePost(const std::string& post_id, FilePost& out);

    int ExtractCredentials(FilePost& in, Credentials& out);
    int ConstructFilepath(const FileInfo& fi, std::string& out);
    void ConstructFilepath(const FileInfo& fi, const Folder& folder, std::string& out);

    int ConstructFile(FileInfo& fi,
                  const Credentials& file_cred,
                  const std::string& destination_path);

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

