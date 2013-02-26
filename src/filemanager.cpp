#include "filemanager.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

#include "utils.h"
#include "chunkinfo.h"

// TODO :: considering removing all file reading/writing out of 
//         manifest to filemanager. (centralize all file writing)
//

static const char* szCompSuffix = "_cmp";
static const char* szEncryptSuffix = "_enc";

FileManager::FileManager() : MutexClass()
{

}

FileManager::FileManager( const std::string &manifestpath, 
                          const std::string &workingDirectory, 
                          unsigned int uFileStride)
{
    // Set manifest path
    m_ManifestFilePath = manifestpath;
    m_Manifest.SetFilePath(m_ManifestFilePath);

    // Set working directory
    m_WorkingDirectory = workingDirectory;

    // Set File Stride
    m_FileStride = uFileStride;
    m_Chunker.SetChunkSize(m_FileStride);
    m_Crypto.SetStride(m_FileStride);
}

FileManager::~FileManager()
{

}

bool FileManager::StartupFileManager()
{
    m_Manifest.Initialize();
    return true;
}

bool FileManager::ShutdownFileManager()
{
    m_Manifest.Shutdown();
    return true;
}



FileInfo* FileManager::CreateFileInfo( const std::string &filename,
                                       const std::string &filepath,
                                       const std::string &chunkName,
                                       const std::string &chunkCount,
                                       const std::string &fileSize,
                                       const std::string &postId,
                                       const std::string &postVersion,
                                       unsigned char *key, // byte
                                       unsigned char *iv) // byte
{
    char* pKey = reinterpret_cast<char*>(key);
    char* pIv = reinterpret_cast<char*>(iv);

    FileInfo* pFi = CreateFileInfo( filename,
                                    filepath,
                                    chunkName,
                                    chunkCount,
                                    fileSize,
                                    postId,
                                    postVersion,
                                    std::string(pKey),
                                    std::string(pIv)
                                 );
   return pFi;
}


FileInfo* FileManager::CreateFileInfo( const std::string &filename,
                                       const std::string &filepath,
                                       const std::string &chunkName,
                                       const std::string &chunkCount,
                                       const std::string &fileSize,
                                       const std::string &postId,
                                       const std::string &postVersion,
                                       const std::string &key,
                                       const std::string &iv)
{
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();

    // Filename (str)
    pFi->SetFilename(filename);
    // Filepath (str)
    pFi->SetFilepath(filepath);
    // ChunkName (str)
    pFi->SetChunkName(chunkName);
    // ChunkCount (unsigned int)
    pFi->SetChunkCount((unsigned)atoi(chunkCount.c_str()));
    // FileSize (unsigned int)
    pFi->SetFileSize((unsigned)atoi(fileSize.c_str()));
    // Post ID
    pFi->SetPostID(postId);
    // Post Version
    pFi->SetPostVersion((unsigned)atoi(postVersion.c_str()));
    // Key 
    pFi->SetEncryptedKey(key);
    // Iv
    pFi->SetIv(iv);

    return pFi;
}

int FileManager::RemoveFile(const std::string &filepath)
{
    int status = ret::A_OK;

    Lock();
    if(!m_Manifest.RemoveFileInfo(filepath))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    Unlock();

    return status;
}

void FileManager::SetFilePostId(const std::string &filepath, const std::string& postid)
{
    Lock();
    m_Manifest.InsertFilePostID(filepath, postid);
    Unlock();
}

void FileManager::SetFileChunkPostId(const std::string &filepath, const std::string& postid)
{
    m_Manifest.InsertFileChunkPostID(filepath, postid);
}

FileInfo* FileManager::CreateFileInfo()
{
    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    Unlock();

    return pFi;
}

FileInfo* FileManager::GetFileInfo(const std::string &filepath)
{
    Lock();
    FileInfo* pFi = m_FileInfoFactory.CreateFileInfoObject();
    m_Manifest.QueryForFile(filepath, pFi);
    Unlock();

    if(pFi) {
        if(!pFi->IsValid())
            return NULL;
    }

    return pFi;
}

int FileManager::GetAllFileInfo(std::vector<FileInfo>& out)
{
    Lock();
    int status = m_Manifest.QueryAllFiles(out);
    Unlock();
    return status;
}

