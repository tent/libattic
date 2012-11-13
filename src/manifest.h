

#ifndef MANIFEST_H_
#define MANIFEST_H_
#pragma once

#include <string>

class Manifest
{
public:
    Manifest();
    ~Manifest();
    
private:
    std::string     m_szFilename;

    unsigned int    m_unChunkCount;
    unsigned int    m_unFileSize;

    // TODO :: Check sums ?
    //
};

#endif

