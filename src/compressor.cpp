
#include "compressor.h"

#include <zlib.h>
#include <gzip.h>
#include <files.h>
#include <mqueue.h>
#include <channels.h>


Compressor::Compressor()
{

}

Compressor::~Compressor()
{


}

ret::eCode Compressor::CompressFile(std::string &szFilePath, std::string &szOutputPath, int nDeflatedLevel)
{
    // Pass in path to file needing compressing, output path is the compressed file's destination
    // deflate level (1-9) level of compression

    // TODO :: it'd be nice for some sort of filepath validation.
    //
    try // Cryptopp can throw exceptions
    {
        // Use filter to check integrity after compression    
        CryptoPP::EqualityComparisonFilter comparison;
        CryptoPP::ChannelSwitch *compSwitch = new CryptoPP::ChannelSwitch(comparison, "0");
        // gunzip takes ownership of the data, and will free it later.
        CryptoPP::Gunzip gz(compSwitch);
        gz.SetAutoSignalPropagation(0);
        
        CryptoPP::FileSink sink(szOutputPath.c_str());
        
        CryptoPP::ChannelSwitch *cs = new CryptoPP::ChannelSwitch(sink);
        // gzip takes ownership of the data, and will free it later.
        CryptoPP::Gzip gzip(cs, nDeflatedLevel);
        cs->AddDefaultRoute(gz);
        
        cs = new CryptoPP::ChannelSwitch(gzip);
        cs->AddDefaultRoute(comparison, "1");

        CryptoPP::FileSource source(szFilePath.c_str(), true, cs);
       
        comparison.ChannelMessageSeriesEnd("0");
        comparison.ChannelMessageSeriesEnd("1");
    }
    catch(std::exception &e)
    {
        // Some sort of logging
        std::cerr << e.what() << std::endl;
        return ret::A_FAIL_COMPRESS;
    }
    
    return ret::A_OK;
}

ret::eCode Compressor::DecompressFile(std::string &szFilePath, std::string &szOutputPath)
{
    try
    {
        CryptoPP::FileSource(szFilePath.c_str(), true, new CryptoPP::Gunzip(new CryptoPP::FileSink(szOutputPath.c_str())));
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return ret::A_FAIL_DECOMPRESS;
    }

    return ret::A_OK;
}



