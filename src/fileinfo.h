
#ifndef FILEINFO_H_
#define FILEINFO_H_
#pragma once

#include <string>
#include <deque>

#include "crypto.h"

class FileInfo
{
    std::string ExtractFileName(std::string &szFilePath);
public:
    FileInfo();
    ~FileInfo();

    // Init a file
    bool InitializeFile(std::string &szFilePath);
    
    void SetFileName(std::string &szFileName) { m_FileName = szFileName; }
    void SetFilePath(std::string &szFilePath) { m_FilePath = szFilePath; }
    void SetChunkName(std::string &szChunkName) { m_ChunkName = szChunkName; }
    void SetChunkCount(unsigned int unChunkCount) { m_ChunkCount = unChunkCount; }
    void SetFileSize(unsigned int unFileSize) { m_FileSize = unFileSize; }
    void SetCredentials(Credentials &tCred) { m_Credentials = tCred; }
    
    std::string GetFileName() const { return m_FileName; }
    std::string GetFilePath() const { return m_FilePath; }
    std::string GetChunkName() const { return m_ChunkName; }
    unsigned int GetChunkCount() const { return m_ChunkCount; }
    unsigned int GetFileSize() const { return m_FileSize; }
    Credentials GetCredentials() const { return m_Credentials; }

private:    
    Credentials     m_Credentials;
    std::string     m_FileName;   // File within directory
    std::string     m_FilePath;   // Directory
    std::string     m_ChunkName;

    unsigned int    m_ChunkCount;
    unsigned int    m_FileSize;   // Filesize, not compressed
};


class FileInfoFactory
{
public:
    FileInfoFactory();
    ~FileInfoFactory();

    FileInfo* CreateFileInfoObject();

private:
    std::deque<FileInfo*> m_FileList; // List of FileInfo objects created.

};

#endif

