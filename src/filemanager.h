

#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <string>

#include "manifest.h"
#include "chunker.h"

class Filemanager
{

public:
    Filemanager();
    ~Filemanager();

    void StartupFileManager();
    void ShutdownFileManager();

    void IndexFile(std::string &szFilePath);

private:
    Manifest    m_Manifest;
    Chunker     m_Chunker;
};


#endif
