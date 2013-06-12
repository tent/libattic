#ifndef FILEROLLER_H_
#define FILEROLLER_H_
#pragma once

#include <string>
#include <fstream>

namespace attic { 
class FileRoller {
public:
    FileRoller(const std::string& filepath) {
        filepath_ = filepath;
    }
    ~FileRoller() {}

    bool Digest(std::string& out) {
        static int window = 1000000;
        bool ret = false;
        std::ifstream ifs;
        ifs.open(filepath_.c_str(), std::ios::in | std::ios::binary);
        if(ifs.is_open()) {
            ifs.seekg (0, std::ios::end);
            unsigned int size = ifs.tellg();
            ifs.seekg (0, std::ios::beg);
            unsigned int bytes_read = 0 ;

            unsigned char hash_out[32];
            crypto_generichash_state st; // state
            crypto_generichash_init(&st, NULL, NULL, 32);

            char* data = NULL;
            std::cout<<" size : " << size << std::endl;
            while((size - bytes_read) > 0) { 
                unsigned int read = 0;
                if((size - bytes_read) < window) {
                    read = (size-bytes_read);
                    data = new char[read];
                    ifs.read(data, read);
                    bytes_read += read;
                }
                else {
                    read = window;
                    data = new char[window];
                    ifs.read(data, window);
                    bytes_read += window;
                }

                int us = crypto_generichash_update(&st, reinterpret_cast<const unsigned char*>(data), read);

                if(data) {
                    delete[] data;
                    data = NULL;
                }
            }
            ifs.close();
            crypto_generichash_final(&st, hash_out, 32);
            std::string hash_b;
            out.append(reinterpret_cast<const char*>(hash_out), 32);
            ret = true;
        }
        return ret;
    }
private:
    std::string filepath_;
};

} //namespace
#endif

