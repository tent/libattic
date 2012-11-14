
#ifndef FILEINFO_H_
#define FILEINFO_H_
#pragma once

#include <string>

class FileInfo
{
    bool CheckFileSize();
public:
    FileInfo();
    ~FileInfo();

    // Init a file
    bool InitializeFile(std::string &szFilePath);
    
    void SetFileName(std::string &szFileName) { m_fileName = szFileName; }
    void SetFilePath(std::string &szFilePath) { m_filePath = szFilePath; }
    void SetChunkName(std::string &szChunkName) { m_chunkName = szChunkName; }
    void SetChunkCount(unsigned int unChunkCount) { m_chunkCount = unChunkCount; }
    
    std::string GetFileName() { return m_fileName; }
    std::string GetFilePath() { return m_filePath; }
    std::string GetChunkName() { return m_chunkName; }

    unsigned int GetChunkCount() { return m_chunkCount; }
    unsigned int GetFileSize() { return m_fileSize; }

private:    
    std::string     m_fileName;   // File within directory
    std::string     m_filePath;   // Directory
    std::string     m_chunkName;

    unsigned int    m_chunkCount;
    unsigned int    m_fileSize;   // Filesize, not compressed
};

#endif

