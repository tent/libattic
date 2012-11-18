#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <fstream>
#include <string>

#include "manifest.h"
#include "chunker.h"
#include "fileinfo.h"
#include "compressor.h"
#include "crypto.h"
#include "errorcodes.h"

class FileManager
{
    FileInfo* CreateFileInfo();
    bool ReadInHeader(std::string &h);
    bool ReadInEntry(std::string &e);

    std::string ConstructOutboundPath(std::string &szWorkingDir, bool bStripFileType, std::string &szFileName, std::string &szPostfix);
    
    void GenerateCompressionPath(FileInfo* fi, std::string &szOutPath);
    void GenerateCryptoPath(FileInfo* fi, std::string &szOutPath);

    FileManager(const FileManager &rhs) { }
    FileManager operator=(const FileManager &rhs) { return *this; }
public:
    FileManager();
    FileManager(std::string &szManifestFilepath, std::string &szWorkingDirectory, unsigned int uFileStride = 400000);
    ~FileManager();

    bool StartupFileManager();
    bool ShutdownFileManager();

    bool LoadManifest(std::string &szFilePath);

    ret::eCode IndexFile(std::string &szFilePath);
    ret::eCode ConstructFile(std::string &szFileName);

    bool FileExists(std::string& szFilepath);

    std::string GetManifestFilePath() { return m_ManifestFilePath; }
    std::string GetWorkingDirectory() { return m_WorkingDirectory; }
    unsigned int GetFileStride()      { return m_FileStride; }

    void SetManifestFilePath(std::string &szFilepath)   { m_ManifestFilePath = szFilepath; }
    void SetWorkingDirectory(std::string &szWorkingDir) { m_WorkingDirectory = szWorkingDir; }
    void SetFileStride(unsigned int uFileStride )       { m_FileStride = uFileStride; }

private:
    FileInfoFactory     m_FileInfoFactory;
    Manifest            m_Manifest;
    Chunker             m_Chunker;
    Crypto              m_Crypto;
    Compressor          m_Compressor;

    std::ifstream       m_ifStream;
    std::ofstream       m_ofStream;

    std::string         m_ManifestFilePath; // Location of manifest
    std::string         m_WorkingDirectory; // Location where file copies will be made to
                                            // and file operations will happen. (ie : crypto,
                                            // compression, chunking, etc ...)
                                            
    unsigned int        m_FileStride;       // Generic file stride to be used by chunker,  
                                            // compressor, and crypto
};


#endif
