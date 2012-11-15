

#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <fstream>
#include <string>

#include "manifest.h"
#include "chunker.h"
#include "fileinfo.h"


class FileManager
{
    FileInfo* CreateFileInfo();
    bool ReadInHeader(std::string &h);
    bool ReadInEntry(std::string &e);

public:
    FileManager(std::string &szManifestFilepath);
    ~FileManager();

    bool StartupFileManager();
    bool ShutdownFileManager();

    bool LoadManifest(std::string &szFilePath);

    bool IndexFile(std::string &szFilePath);
    bool ConstructFile(std::string &szFileName);

    bool FileExists(std::string& szFilepath);

    std::string GetManifestFilePath() { return m_ManifestFilePath; }

    void SetManifestFilePath(std::string &szFilepath) { m_ManifestFilePath = szFilepath; }

private:
    FileInfoFactory     m_FileInfoFactory;
    Manifest            m_Manifest;
    Chunker             m_Chunker;

    std::ifstream       m_ifStream;
    std::ofstream       m_ofStream;


    std::string         m_ManifestFilePath; // Location of manifest
};


#endif
