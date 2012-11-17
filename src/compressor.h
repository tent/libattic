

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

    ret::eCode CompressFile(std::string &szFilePath, std::string &szOutputPath, int nDeflatedLevel);
    ret::eCode DecompressFile(std::string &szFilePath, std::string &szOutputPath);


private:

};

#endif
