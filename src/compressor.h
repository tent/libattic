

#ifndef COMPRESSOR_H_
#define COMPRESSOR_H_
#pragma once

#include <string>
#include "errorcodes.h"

class Compressor
{
public:
    Compressor();
    ~Compressor();

    ret::eCode CompressFile( const std::string &szFilePath, 
                             std::string &szOutputPath, 
                             int nDeflatedLevel);

    ret::eCode DecompressFile( const std::string &szFilePath, 
                               std::string &szOutputPath);


private:

};

#endif
