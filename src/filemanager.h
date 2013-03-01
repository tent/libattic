#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <fstream>
#include <string>

#include "mutexclass.h"
#include "manifest.h"
#include "fileinfo.h"
#include "fileinfofactory.h"
#include "errorcodes.h"

class FileManager : public MutexClass
{
    FileManager(const FileManager &rhs) { }
    FileManager operator=(const FileManager &rhs) { return *this; }
public:
    FileManager();
    FileManager( const std::string &manifestDirectory, 
                 const std::string &workingDirectory, 
                 unsigned int uFileStride = 400000);

    ~FileManager();

    bool StartupFileManager();
    bool ShutdownFileManager();

    int RemoveFile(const std::string &filepath);

    FileInfo* GetFileInfo(const std::string &filepath);

    int GetAllFileInfo(std::vector<FileInfo>& out);


    FileInfo* CreateFileInfo();

    void InsertToManifest (FileInfo* pFi);

    void GetManifestDirectory(std::string &out) const { out = m_ManifestDirectory; }
    void GetWorkingDirectory(std::string &out) const { out = m_WorkingDirectory; }
    void GetTempDirectory(std::string &out) const    { out = m_TempDirectory; }
    unsigned int GetFileStride() const               { return m_FileStride; }

    void SetManifestDirectory(const std::string &filepath)       { m_ManifestDirectory = filepath; }
    void SetWorkingDirectory(const std::string &workingDir)     { m_WorkingDirectory = workingDir; }
    void SetTempDirectory(const std::string &tempDir)           { m_TempDirectory = tempDir; }
    void SetFileStride(unsigned int uFileStride )               { m_FileStride = uFileStride; }
    void SetFilePostId(const std::string &filepath, const std::string& postid);
    void SetFileChunkPostId(const std::string &filepath, const std::string& postid);

private:
    FileInfoFactory     m_FileInfoFactory;
    Manifest            m_Manifest;

    std::string         m_ManifestDirectory; // Location of manifest
    std::string         m_WorkingDirectory; // Location where original files live.
    std::string         m_TempDirectory;    // Location where file copies will be made and manipulated
                                            // compression, chunking, cryptio, etc ...
                                            
    unsigned int        m_FileStride;       // Generic file stride to be used by chunker,  
                                            // compressor, and crypto
};


#endif
