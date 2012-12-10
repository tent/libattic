
#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <time.h>
#include <stdlib.h>

namespace utils
{

    typedef std::vector<std::string> split;
    static std::vector<std::string> &SplitString(const std::string &s, char delim, split &out)
    {
        std::stringstream ss(s);
        std::string item;

        while(std::getline(ss, item, delim))
        {
            out.push_back(item);
        }

        return out;
    }

    static unsigned int CheckFileSize(const std::string &filepath)
    {
        unsigned int fileSize = 0;

        std::ifstream ifs;
        ifs.open (filepath.c_str(), std::ifstream::binary);

        if(ifs.is_open())
        {
           ifs.seekg (0, std::ifstream::end);
           fileSize = ifs.tellg();
           ifs.seekg (0, std::ifstream::beg);
           ifs.close();
        }

        return fileSize;
    }

    static unsigned int CheckIStreamSize(std::ifstream &ifs)
    {
        unsigned int size = 0;
        ifs.seekg(0, std::ifstream::end);
        size = ifs.tellg();
        ifs.seekg(0, std::ifstream::beg);
        
        return size;
    };

    static void StringToHex(const std::string& input, std::string& output)
    {
        static const char* const lut = "0123456789ABCDEF";
        size_t len = input.length();

        output.clear();
        output.reserve(2 * len);
        for (size_t i = 0; i < len; ++i)
        {
            const char c = input[i];
            output.push_back(lut[c >> 4]);
            output.push_back(lut[c & 15]);
        }
    }

    static char GenerateChar()
    {
        char c;
        if(rand()%2)
            c = rand()%25 + 97; // a-z
        else
            c = rand()%9 + 48; // 0-9

        return c;
    }

    static void SeedRand()
    {
        srand(time(0));
    }

    static void ExtractFileName(const std::string& filepath, std::string& out)
    {
        unsigned int size = filepath.size();                     
        if(size)                                                   
        {                                                          
            // Check if passed a directory                         
            if(filepath[size-1] == '/')                          
                return;                                       

            std::vector<std::string> split;                          
            utils::SplitString(filepath, '/', split);              
            if(split.size())                                         
            {                                                      
                out = split[split.size()-1];                          
            }                                                      
         }                                                          
    }

    static bool CheckAndRemoveRelativePath(const std::string &filepath, std::string &out)
    {
        if(filepath.size())
        {
            if(filepath[0] == '.')
            {
                out = filepath;
                out.erase(0,1);
                return true;
            }
        }
        return false;
    }

    static void CheckUrlAndAppendTrailingSlash(std::string &url)
    {
        if(url.empty())                                 
            return;                                          

        if(url[url.size()-1] != '/')               
            url.append("/");                            
    }

}
#endif
