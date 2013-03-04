#ifndef COMPRESSION_H_
#define COMPRESSION_H_
#pragma once

#include <gzip.h>
#include <mqueue.h>
#include <channels.h>

namespace compress
{
    static int CompressString(const std::string& in, std::string& out, const int nDeflateLevel = 1)
    {
        // deflate level (1-9) level of compression
        int status = ret::A_OK;
        try {
            // Use filter to check integrity after compression    
            CryptoPP::EqualityComparisonFilter comparison;
            CryptoPP::ChannelSwitch *compSwitch = new CryptoPP::ChannelSwitch(comparison, "0");

            // gunzip takes ownership of the data, and will free it later.
            CryptoPP::Gunzip gz(compSwitch);
            gz.SetAutoSignalPropagation(0);

            CryptoPP::StringSink sink(out);

            CryptoPP::ChannelSwitch *cs = new CryptoPP::ChannelSwitch(sink);
            // gzip takes ownership of the data, and will free it later.
            CryptoPP::Gzip gzip(cs, nDeflateLevel);
            cs->AddDefaultRoute(gz);
            
            cs = new CryptoPP::ChannelSwitch(gzip);
            cs->AddDefaultRoute(comparison, "1");

            CryptoPP::StringSource source(in, true, cs);
       
            comparison.ChannelMessageSeriesEnd("0");
            comparison.ChannelMessageSeriesEnd("1");
        }
        catch(std::exception &e) {
            // Some sort of logging
            std::cerr << e.what() << std::endl;
            status = ret::A_FAIL_COMPRESS;
        }

        return status;
    }

    static int DecompressString(std::string& in, std::string& out)
    {
        int status = ret::A_OK;
        try {
            CryptoPP::StringSource( in,
                                    true,
                                    new CryptoPP::Gunzip(new CryptoPP::StringSink(out)));
        }
        catch(std::exception &e) {
            // Some sort of logging
            std::cerr << e.what() << std::endl;
            status = ret::A_FAIL_COMPRESS;
        }
        return status;
    }

};


#endif

