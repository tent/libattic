#ifndef UTILS_H_
#define UTILS_H_

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include <time.h>
#include <stdlib.h>

namespace attic { namespace utils {
typedef std::vector<std::string> split;
static std::vector<std::string> &SplitString( const std::string &s, 
                                              char delim, 
                                              split &out)
{
    std::stringstream ss(s);
    std::string item;

    while(std::getline(ss, item, delim)) {
        out.push_back(item);
    }

    return out;
}

static std::vector<std::string> &SplitStringSubStr( const std::string& s,
                                              const std::string& delim,
                                              split& out)
{
    int left = 0;
    int right = 0; 
    std::string hold;
    for(;;) {
        hold.clear();
        right = s.find(delim, right);
        if(right == std::string::npos) // npos == -1
            break;

        int diff = right - left;
        hold = s.substr(left, diff);
        out.push_back(hold);
        left = right + delim.size();
        right = left;
    }

    return out;
}

static unsigned int CheckFilesize(const std::string &filepath) {
    unsigned int fileSize = 0;

    std::ifstream ifs;
    ifs.open (filepath.c_str(), std::ifstream::binary);

    if(ifs.is_open()) {
       ifs.seekg (0, std::ifstream::end);
       fileSize = ifs.tellg();
       ifs.seekg (0, std::ifstream::beg);
       ifs.close();
    }

    return fileSize;
}

static bool CheckFileExists(const std::string &filepath) {
    bool bExists = false;

    std::ifstream ifs;
    ifs.open(filepath.c_str(), std::ifstream::binary);

    if(ifs.is_open()) {
        bExists = true;
        ifs.close();
    }

    return bExists;
}

static unsigned int CheckIStreamSize(std::ifstream &ifs) {
    unsigned int size = 0;
    ifs.seekg(0, std::ifstream::end);
    size = ifs.tellg();
    ifs.seekg(0, std::ifstream::beg);
    
    return size;
};

static void StringToHex(const std::string& input, std::string& output) {
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    output.clear();
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        const char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
}

static void HexToString(const std::string& input, std::string& output) {
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    if (len & 1) throw std::invalid_argument("odd length");

    output.clear();
    output.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2) {
        char a = input[i];
        const char* p = std::lower_bound(lut, lut + 16, a);
        if (*p != a) throw std::invalid_argument("not a hex digit");

        char b = input[i + 1];
        const char* q = std::lower_bound(lut, lut + 16, b);
        if (*q != b) throw std::invalid_argument("not a hex digit");

        output.push_back(((p - lut) << 4) | (q - lut));
    }
}

static char GenerateChar() {
    char c;
    if(rand()%2)
        c = rand()%25 + 97; // a-z
    else
        c = rand()%9 + 48; // 0-9

    return c;
}

static void SeedRand() {
    srand(time(0));
}

static void ExtractFileName(const std::string& filepath, std::string& out) {
    unsigned int size = filepath.size();
    if(size) {
        // Check if passed a directory                         
        if(filepath[size-1] != '/') {
            std::vector<std::string> split;
            utils::SplitString(filepath, '/', split);
            if(split.size()) {
                out = split[split.size()-1];
            }
        }
     }
}

static bool CheckAndRemoveRelativePath(const std::string &filepath, std::string &out) {
    if(filepath.size()) {
        if(filepath[0] == '.') {
            out = filepath;
            out.erase(0,1);
            return true;
        }
    }
    return false;
}

static void CheckUrlAndAppendTrailingSlash(std::string &url) {
    if(url.empty())                                 
        return;                                          

    if(url[url.size()-1] != '/')               
        url.append("/");                            
}

static void CheckUrlAndRemoveTrailingSlash(std::string &url) {
    if(url.empty())                                 
        return;                                          

    if(url[url.size()-1] == '/') {
        size_t end = (url.size()-1)-0;
        std::string hold = url.substr(0, end);
        url.clear();
        url += hold;
    }
}

static void GenerateRandomString(std::string& out, unsigned int len = 10) {
    static const char alph[] = { "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz"};

    unsigned int size = sizeof(alph)-1;

    for(unsigned int i=0; i<len; i++) {
        out += alph[rand()%size];
    }
}

typedef std::vector<std::string> taglist;
static void FindAndExtractAllTags( const std::string& tag, 
                                   const std::string& content, 
                                   taglist& out)
{
    size_t found;

    std::string btag;
    btag += "<";
    btag += tag;

    found = content.find(btag);
    std::string str;
    while(found != std::string::npos) {
        // Extract tag
        // Find the end of the tag
        size_t pos = content.find("/>", found);
        size_t dif = ((pos-found)+2);

        //if(dif < 0) // dif is unsigned so this is a useless check
        //    break; // TODO:: perhaps log an error
       
        str.clear();
        str = content.substr(found, dif);
        
        // Push back on vector
        out.push_back(str);
        
        // Look for the next tag
        found = content.find(btag, pos);
    }
} 

static void ExtractFromHeader( const std::string& delim,
                               const std::string& content,
                               std::vector<std::string>& out)
{
    std::cout<<" incoming : " << content << std::endl;
    size_t found = content.find(delim);
    std::cout<<" found : " << found << std::endl;
} 

static void FindAndReplace(const std::string& in,
                           const std::string& delim, 
                           const std::string& replace_with,
                           std::string& out) {
    out = in;
    for(size_t pos = out.find(delim); pos != std::string::npos; pos = out.find(delim)) {
        out.erase(pos, delim.size());
        out.insert(pos, replace_with);
    }
}

static void ExtractSubPaths(const std::string& root, 
                            const std::string& filepath,
                            std::deque<std::string>& out) {
    if(filepath.find(root) != std::string::npos) {
        unsigned int filepath_size = filepath.size();
        if(filepath_size) {
            std::string filename;
            ExtractFileName(filepath, filename);
            // erase filename
            std::string local_filepath = filepath;
            size_t fn_pos = filepath.find(filename);
            local_filepath.erase(fn_pos-1, filename.size()+1);

            size_t pos = std::string::npos;
            while(local_filepath != root) {
                std::cout<< "LOCAL FILEPATH : " << local_filepath << std::endl;
                out.push_back(local_filepath);
                pos = local_filepath.rfind("/");
                if(pos != std::string::npos) {
                    std::string end = local_filepath.substr(pos);
                    local_filepath.erase(pos, end.size());
                }
            }
        }
    }
}
// depricated
static void ExtractSubPaths(const std::string& root, 
                            const std::string& filepath,
                            std::vector<std::string>& out) {
    if(filepath.find(root) != std::string::npos) {
        unsigned int filepath_size = filepath.size();
        if(filepath_size) {
            std::string filename;
            ExtractFileName(filepath, filename);
            // erase filename
            std::string local_filepath = filepath;
            size_t fn_pos = filepath.find(filename);
            local_filepath.erase(fn_pos-1, filename.size()+1);

            size_t pos = std::string::npos;
            while(local_filepath != root) {
                std::cout<< "LOCAL FILEPATH : " << local_filepath << std::endl;
                out.push_back(local_filepath);
                pos = local_filepath.rfind("/");
                if(pos != std::string::npos) {
                    std::string end = local_filepath.substr(pos);
                    local_filepath.erase(pos, end.size());
                }
            }
        }
    }
}

}}//namespace
#endif

